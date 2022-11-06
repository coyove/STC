// Implements c++ example: https://en.cppreference.com/w/cpp/container/priority_queue
// Example of per-instance less-function on a single priority queue type
//
// Note: i_less_functor: available for cpque types
//       i_cmp_functor: available for csmap and csset types
//       i_hash_functor/i_eq_functor: available for cmap and cset types

#include <stdio.h>
#include <stdbool.h>
#include <stc/forward.h>
#include <stc/views.h>

// predeclare
declare_cpque(ipque, int);

struct {
    ipque Q;
    bool (*less)(const int*, const int*);
} typedef IPQueue;

#define IPQueue_obj(less) ((IPQueue){ipque_init(), less})

#define i_type ipque
#define i_val int
#define i_opt c_declared // needed to avoid re-type-define container type
#define i_less_functor(self, x, y) c_container_of(self, IPQueue, Q)->less(x, y) // <== This.
#include <stc/cpque.h>

#define print(name, q, n) do { \
    printf("%s: \t", name); \
    c_forrange (i, n) printf("%d ", q[i]); \
    puts(""); \
} while(0)

void print_queue(const char* name, IPQueue q) {
    // NB: make a clone because there is no way to traverse
    // priority_queue's content without erasing the queue.
    IPQueue copy = {ipque_clone(q.Q), q.less};
    
    for (printf("%s: \t", name); !ipque_empty(&copy.Q); ipque_pop(&copy.Q))
        printf("%d ", *ipque_top(&copy.Q));
    puts("");
    ipque_drop(&copy.Q);
}

static bool int_less(const int* x, const int* y) { return *x < *y; }
static bool int_greater(const int* x, const int* y) { return *x > *y; }
static bool int_lambda(const int* x, const int* y) { return (*x ^ 1) < (*y ^ 1); }

int main()
{
    const int data[] = {1,8,5,6,3,4,0,9,7,2}, n = c_arraylen(data);
    print("data", data, n);

    c_with (IPQueue q1 = IPQueue_obj(int_less), ipque_drop(&q1.Q))   // Max priority queue
    c_with (IPQueue minq1 = IPQueue_obj(int_greater), ipque_drop(&minq1.Q)) // Min priority queue
    c_with (IPQueue q5 = IPQueue_obj(int_lambda), ipque_drop(&q5.Q)) // Using lambda to compare elements.
    {
        c_forrange (i, n)
            ipque_push(&q1.Q, data[i]);
        print_queue("q1", q1);

        c_forrange (i, n)
            ipque_push(&minq1.Q, data[i]);
        print_queue("minq1", minq1);

        c_forrange (i, n)
            ipque_push(&q5.Q, data[i]);
        print_queue("q5", q5);
    }
}
