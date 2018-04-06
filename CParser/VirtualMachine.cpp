#include "stdafx.h"
#include "VirtualMachine.h"

int g_argc;
int g_argv;

#define LOG 1
#define INC_PTR 4
#define VMM_ARG(s, p) ((s) + vmm_get((p) + INC_PTR) * INC_PTR)
#define VMM_ARGS(t, n) vmm_get(t - (n) * INC_PTR)

uint32_t CVirtualMachine::pmm_alloc()
{
    auto page = PAGE_ALIGN_UP((uint32_t)memory.alloc_array<byte>(PAGE_SIZE * 2));
    memset((void*)page, 0, PAGE_SIZE);
    return page;
}

void CVirtualMachine::vmm_init() {
    pgd_kern = (pde_t *)malloc(PTE_SIZE * sizeof(pde_t));
    memset(pgd_kern, 0, PTE_SIZE * sizeof(pde_t));
    pte_kern = (pte_t *)malloc(PTE_COUNT*PTE_SIZE * sizeof(pte_t));
    memset(pte_kern, 0, PTE_COUNT*PTE_SIZE * sizeof(pte_t));
    pgdir = pgd_kern;

    uint32_t i;

    // map 4G memory, physcial address = virtual address
    for (i = 0; i < PTE_SIZE; i++) {
        pgd_kern[i] = (uint32_t)pte_kern[i] | PTE_P | PTE_R | PTE_K;
    }

    uint32_t *pte = (uint32_t *)pte_kern;
    for (i = 1; i < PTE_COUNT*PTE_SIZE; i++) {
        pte[i] = (i << 12) | PTE_P | PTE_R | PTE_K; // i是页表号
    }
}

// 虚页映射
// va = 虚拟地址  pa = 物理地址
void CVirtualMachine::vmm_map(uint32_t va, uint32_t pa, uint32_t flags) {
    uint32_t pde_idx = PDE_INDEX(va); // 页目录号
    uint32_t pte_idx = PTE_INDEX(va); // 页表号

    pte_t *pte = (pte_t *)(pgdir[pde_idx] & PAGE_MASK); // 页表

    if (!pte) { // 缺页
        if (va >= USER_BASE) { // 若是用户地址则转换
            pte = (pte_t *)pmm_alloc(); // 申请物理页框，用作新页表
            pgdir[pde_idx] = (uint32_t)pte | PTE_P | flags; // 设置页表
            pte[pte_idx] = (pa & PAGE_MASK) | PTE_P | flags; // 设置页表项
        }
        else { // 内核地址不转换
            pte = (pte_t *)(pgd_kern[pde_idx] & PAGE_MASK); // 取得内核页表
            pgdir[pde_idx] = (uint32_t)pte | PTE_P | flags; // 设置页表
        }
    }
    else { // pte存在
        pte[pte_idx] = (pa & PAGE_MASK) | PTE_P | flags; // 设置页表项
    }
}

// 释放虚页
void CVirtualMachine::vmm_unmap(pde_t *pde, uint32_t va) {
    uint32_t pde_idx = PDE_INDEX(va);
    uint32_t pte_idx = PTE_INDEX(va);

    pte_t *pte = (pte_t *)(pde[pde_idx] & PAGE_MASK);

    if (!pte) {
        return;
    }

    pte[pte_idx] = 0; // 清空页表项，此时有效位为零
}

// 是否已分页
int CVirtualMachine::vmm_ismap(uint32_t va, uint32_t *pa) const {
    uint32_t pde_idx = PDE_INDEX(va);
    uint32_t pte_idx = PTE_INDEX(va);

    pte_t *pte = (pte_t *)(pgdir[pde_idx] & PAGE_MASK);
    if (!pte) {
        return 0; // 页表不存在
    }
    if (pte[pte_idx] != 0 && (pte[pte_idx] & PTE_P) && pa) {
        *pa = pte[pte_idx] & PAGE_MASK; // 计算物理页面
        return 1; // 页面存在
    }
    return 0; // 页表项不存在
}

char* CVirtualMachine::vmm_getstr(uint32_t va)
{
    uint32_t pa;
    if (vmm_ismap(va, &pa))
    {
        return (char*)pa + OFFSET_INDEX(va);
    }
    vmm_map(va, pmm_alloc(), PTE_U | PTE_P | PTE_R);
    assert(0);
    return vmm_getstr(va);
}

template<class T>
T CVirtualMachine::vmm_get(uint32_t va)
{
    uint32_t pa;
    if (vmm_ismap(va, &pa))
    {
        return *(T*)((byte*)pa + OFFSET_INDEX(va));
    }
    vmm_map(va, pmm_alloc(), PTE_U | PTE_P | PTE_R);
    assert(0);
    return vmm_get<T>(va);
}

template<class T>
T CVirtualMachine::vmm_set(uint32_t va, T value)
{
    uint32_t pa;
    if (vmm_ismap(va, &pa))
    {
        *(T*)((byte*)pa + OFFSET_INDEX(va)) = value;
        return value;
    }
    vmm_map(va, pmm_alloc(), PTE_U | PTE_P | PTE_R);
    assert(0);
    return vmm_set(va, value);
}

void CVirtualMachine::vmm_setstr(uint32_t va, const char *value)
{
    auto len = strlen(value);
    for (uint32_t i = 0; i < len; i++)
    {
        vmm_set(va + i, value[i]);
    }
    vmm_set(va + len, '\0');
}

uint32_t vmm_pa2va(uint32_t base, uint32_t size, uint32_t pa)
{
    return base + (pa & (size * PAGE_SIZE - 1));
}

uint32_t CVirtualMachine::vmm_malloc(uint32_t size)
{
    printf("[MALLOC] Available: %08X\n", heap.available() * 0x10);
    auto ptr = heap.alloc_array<byte>(size);
    if (ptr == nullptr)
    {
        printf("out of memory");
        exit(-1);
    }
    if (ptr < heapHead)
    {
        heap.alloc_array<byte>(heapHead - ptr);
        return vmm_malloc(size);
    }
    if (ptr + size >= heapHead + HEAP_SIZE * PAGE_SIZE)
    {
        printf("out of memory");
        exit(-1);
    }
#if 1
    printf("[MALLOC] %p> %08X bytes\n", ptr, size);
#endif
    return vmm_pa2va(HEAP_BASE, HEAP_SIZE, (uint32_t)ptr);
}

uint32_t CVirtualMachine::vmm_memset(uint32_t va, uint32_t value, uint32_t count)
{
    for (uint32_t i = 0; i < count; i++)
    {
        vmm_set(va + i, value);
    }
    return 0;
}

uint32_t CVirtualMachine::vmm_memcmp(uint32_t src, uint32_t dst, uint32_t count)
{
    for (uint32_t i = 0; i < count; i++)
    {
        if (vmm_get<byte>(src + i) > vmm_get<byte>(dst + i))
            return 1;
        if (vmm_get<byte>(src + i) < vmm_get<byte>(dst + i))
            return -1;
    }
    return 0;
}

template <class T>
void CVirtualMachine::vmm_pushstack(uint32_t& sp, T value)
{
    sp -= sizeof(T);
    vmm_set(sp, value);
}

template <class T>
T CVirtualMachine::vmm_popstack(uint32_t& sp)
{
    T t = vmm_get(sp);
    sp += sizeof(T);
    return t;
}


//-----------------------------------------


CVirtualMachine::CVirtualMachine(std::vector<LEX_T(int)> text, std::vector<LEX_T(char)> data)
{
    vmm_init();
    uint32_t pa;
    /* 映射4KB的代码空间 */
    {
        auto size = PAGE_SIZE / sizeof(int);
        for (uint32_t i = 0, start = 0; start < text.size(); ++i, start += size)
        {
            vmm_map(USER_BASE + PAGE_SIZE * i, (uint32_t)pmm_alloc(), PTE_U | PTE_P | PTE_R); // 用户代码空间
            if (vmm_ismap(USER_BASE + PAGE_SIZE * i, &pa))
            {
                auto s = start + size > text.size() ? (text.size() & (size - 1)) : size;
                for (uint32_t j = 0; j < s; ++j)
                {
                    *((uint32_t*)pa + j) = text[start + j];
#if 1
                    printf("[%p]> [%08X] %08X\n", (int*)pa + j, USER_BASE + PAGE_SIZE * i + j * 4, vmm_get<uint32_t>(USER_BASE + PAGE_SIZE * i + j * 4));
#endif
                }
            }
        }
    }
    /* 映射4KB的数据空间 */
    {
        auto size = PAGE_SIZE;
        for (uint32_t i = 0, start = 0; start < data.size(); ++i, start += size)
        {
            vmm_map(DATA_BASE + PAGE_SIZE * i, (uint32_t)pmm_alloc(), PTE_U | PTE_P | PTE_R); // 用户数据空间
            if (vmm_ismap(DATA_BASE + PAGE_SIZE * i, &pa))
            {
                auto s = start + size > data.size() ? (data.size() & (size - 1)) : size;
                for (uint32_t j = 0; j < s; ++j)
                {
                    *((char*)pa + j) = data[start + j];
#if 0
                    printf("[%p]> [%08X] %d\n", (char*)pa + j, DATA_BASE + PAGE_SIZE * i + j, vmm_get<byte>(DATA_BASE + PAGE_SIZE * i + j));
#endif
                }
            }
        }
    }
    /* 映射4KB的栈空间 */
    vmm_map(STACK_BASE, (uint32_t)pmm_alloc(), PTE_U | PTE_P | PTE_R); // 用户栈空间
    /* 映射16KB的堆空间 */
    {
        auto head = heap.alloc_array<byte>(PAGE_SIZE * 5);
        heapHead = head;
        heap.free_array(heapHead); // 得到内存池起始地址
        heapHead = (byte*)PAGE_ALIGN_UP((uint32_t)head);
        memset(heapHead, 0, PAGE_SIZE);
        for (int i = 0; i < HEAP_SIZE; ++i)
        {
            vmm_map(HEAP_BASE + PAGE_SIZE * i, (uint32_t)heapHead + PAGE_SIZE * i, PTE_U | PTE_P | PTE_R);
        }
    }
}

CVirtualMachine::~CVirtualMachine()
{
    free(pgd_kern);
    free(pte_kern);
}

int CVirtualMachine::exec(int entry)
{
    auto poolsize = PAGE_SIZE;
    auto stack = STACK_BASE;
    auto data = DATA_BASE;
    auto base = USER_BASE;

    auto sp = stack + poolsize; // 4KB / sizeof(int) = 1024

    {
        auto argvs = vmm_malloc(2 * INC_PTR);
        auto str = vmm_malloc(2);
        vmm_setstr(str, "\0");
        vmm_set(argvs, str);
        str = vmm_malloc(10);
        vmm_setstr(str, "test.txt");
        vmm_set(argvs + 4, str);

        vmm_pushstack(sp, EXIT);
        auto tmp = sp;
        vmm_pushstack(sp, 2);
        vmm_pushstack(sp, argvs);
        vmm_pushstack(sp, tmp);
    }

    auto pc = USER_BASE + entry * INC_PTR;
    auto ax = 0;
    auto bp = 0;

    auto cycle = 0;
    while (true)
    {
        cycle++;
        auto op = vmm_get(pc); // get next operation code
        pc += INC_PTR;

#if LOG
        assert(op <= EXIT);
        // print debug info
        {
            printf("%04d> [%08X] %02d %.4s", cycle, pc, op,
                &"LEA ,IMM ,JMP ,CALL,JZ  ,JNZ ,ENT ,ADJ ,LEV ,LI  ,SI  ,PUSH,LOAD,"
                "OR  ,XOR ,AND ,EQ  ,NE  ,LT  ,GT  ,LE  ,GE  ,SHL ,SHR ,ADD ,SUB ,MUL ,DIV ,MOD ,"
                "OPEN,READ,CLOS,PRTF,MALC,MSET,MCMP,EXIT"[op * 5]);
            if (op == PUSH)
                printf(" %08X\n", (uint32_t)ax);
            else if (op <= ADJ)
                printf(" %d\n", vmm_get(pc));
            else
                printf("\n");
        }
#endif
        switch (op)
        {
        case IMM:
        {
            ax = vmm_get(pc);
            pc += INC_PTR;
        } /* load immediate value to ax */
        break;
        case LI:
        {
            ax = vmm_get(ax);
        } /* load integer to ax, address in ax */
        break;
        case LOAD:
        {
            ax = data | ((ax * INC_PTR) & (PAGE_SIZE - 1));
        } /* load the value of ax, segment = DATA_BASE */
        break;
        case SI:
        {
            vmm_set(vmm_popstack(sp), ax);
        } /* save integer to address, value in ax, address on stack */
        break;
        case PUSH:
        {
            vmm_pushstack(sp, ax);
        } /* push the value of ax onto the stack */
        break;
        case JMP:
        {
            pc = base + vmm_get(pc) * INC_PTR;
        } /* jump to the address */
        break;
        case JZ:
        {
            pc = ax ? pc + INC_PTR : (base + vmm_get(pc) * INC_PTR);
        } /* jump if ax is zero */
        break;
        case JNZ:
        {
            pc = ax ? (base + vmm_get(pc) * INC_PTR) : pc + INC_PTR;
        } /* jump if ax is zero */
        break;
        case CALL:
        {
            vmm_pushstack(sp, pc + INC_PTR);
            pc = base + vmm_get(pc) * INC_PTR;
        } /* call subroutine */
          /* break;case RET: {pc = (int *)*sp++;} // return from subroutine; */
        break;
        case ENT:
        {
            vmm_pushstack(sp, bp);
            bp = sp;
            sp = sp - vmm_get(pc) * INC_PTR;
            pc += INC_PTR;
        } /* make new stack frame */
        break;
        case ADJ:
        {
            sp = sp + vmm_get(pc) * INC_PTR;
            pc += INC_PTR;
        } /* add esp, <size> */
        break;
        case LEV:
        {
            sp = bp;
            bp = vmm_popstack(sp);
            pc = vmm_popstack(sp);
        } /* restore call frame and PC */
        break;
        case LEA:
        {
            ax = bp + vmm_get(pc);
            pc += INC_PTR;
        } /* load address for arguments. */
        break;
        case OR:
            ax = vmm_popstack(sp) | ax;
            break;
        case XOR:
            ax = vmm_popstack(sp) ^ ax;
            break;
        case AND:
            ax = vmm_popstack(sp) & ax;
            break;
        case EQ:
            ax = vmm_popstack(sp) == ax;
            break;
        case NE:
            ax = vmm_popstack(sp) != ax;
            break;
        case LT:
            ax = vmm_popstack(sp) < ax;
            break;
        case LE:
            ax = vmm_popstack(sp) <= ax;
            break;
        case GT:
            ax = vmm_popstack(sp) > ax;
            break;
        case GE:
            ax = vmm_popstack(sp) >= ax;
            break;
        case SHL:
            ax = vmm_popstack(sp) << ax;
            break;
        case SHR:
            ax = vmm_popstack(sp) >> ax;
            break;
        case ADD:
            ax = vmm_popstack(sp) + ax;
            break;
        case SUB:
            ax = vmm_popstack(sp) - ax;
            break;
        case MUL:
            ax = vmm_popstack(sp) * ax;
            break;
        case DIV:
            ax = vmm_popstack(sp) / ax;
            break;
        case MOD:
            ax = vmm_popstack(sp) % ax;
            break;
            // --------------------------------------
        case PRTF:
        {
            auto tmp = VMM_ARG(sp, pc); /* 利用之后的ADJ清栈指令知道函数调用的参数个数 */
            ax = printf(vmm_getstr(VMM_ARGS(tmp, 1)), VMM_ARGS(tmp, 2), VMM_ARGS(tmp, 3), VMM_ARGS(tmp, 4), VMM_ARGS(tmp, 5), VMM_ARGS(tmp, 6));
        }
        break;
        case EXIT:
        {
            printf("exit(%d)\n", ax);
            return ax;
        }
        break;
        case OPEN:
        {
            auto tmp = VMM_ARG(sp, pc);
            ax = (int)fopen(vmm_getstr(VMM_ARGS(tmp, 1)), "r");
        }
        break;
        case READ:
        {
            auto tmp = VMM_ARG(sp, pc);
            ax = (int)fread((void*)VMM_ARGS(tmp, 2), VMM_ARGS(tmp, 3), 1, (FILE*)VMM_ARGS(tmp, 1));
        }
        break;
        case CLOS:
        {
            auto tmp = VMM_ARG(sp, pc);
            ax = (int)fclose((FILE*)VMM_ARGS(tmp, 1));
        }
        break;
        case MALC:
        {
            auto tmp = VMM_ARG(sp, pc);
            ax = (int)vmm_malloc(VMM_ARGS(tmp, 1));
        }
        break;
        case MSET:
        {
            auto tmp = VMM_ARG(sp, pc);
            ax = (int)vmm_memset(VMM_ARGS(tmp, 1), VMM_ARGS(tmp, 2), VMM_ARGS(tmp, 3));
        }
        break;
        case MCMP:
        {
            auto tmp = VMM_ARG(sp, pc);
            ax = (int)vmm_memcmp(VMM_ARGS(tmp, 1), VMM_ARGS(tmp, 2), VMM_ARGS(tmp, 3));
        }
        break;
        default:
            {
                printf("unknown instruction:%d\n", op);
                assert(0);
                exit(-1);
            }
        }

#if 0
    {
    printf("\n---------------- STACK BEGIN <<<< \n");
    printf("AX: %08X BP: %08X SP: %08X\n", ax, bp, sp);
    for (uint32_t i = sp; i < STACK_BASE + PAGE_SIZE; i += 4)
    {
        printf("[%08X]> %08X\n", i, vmm_get<uint32_t>(i));
    }
    printf("---------------- STACK END >>>>\n\n");
    }
#endif
    }
    return 0;
}
