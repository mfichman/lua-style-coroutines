
#include <iostream>
#include <vector>
#include <cstring>
#include <ucontext.h>

class Coroutine {

public:
    typedef void (*Function)(void);
    enum Status { NEW, RUNNING, FINISHED };

    Coroutine(Function function) : 
        status_(NEW),
        stack_(4096) {
        
        getcontext(&context_);
        context_.uc_stack.ss_sp = &stack_[0]; 
        context_.uc_stack.ss_size = stack_.size();
        context_.uc_link = 0;
        makecontext(&context_, function, 0);
    }

    void resume() {
        if (FINISHED == status_) {
            return;
        }
        if (NEW == status_) {
            status_ = RUNNING;
        }
        
        ucontext_t save = caller_;

        // The caller context becomes the current context
        swapcontext(&caller_, &context_);
        context_ = callee_;

        caller_ = save;
    }

    static void yield() {
        // The current context becomes the caller
        swapcontext(&callee_, &caller_);
    }

    static ucontext_t caller_;
    static ucontext_t callee_;

private:
    Status status_;
    ucontext_t context_;
    std::vector<char> stack_;

};

ucontext_t Coroutine::caller_;
ucontext_t Coroutine::callee_;

void g() {
    std::cout << "x" << std::endl;
    Coroutine::yield();
    std::cout << "z" << std::endl;
}

void f() {
    Coroutine co(g);
    co.resume();

    std::cout << "a" << std::endl;
    Coroutine::yield();
    std::cout << "b" << std::endl;
    Coroutine::yield();
    std::cout << "c" << std::endl;
    
    co.resume();
}

int main(int argc, char** argv) {
    Coroutine co(f);
    co.resume();
    co.resume();
    co.resume();
    std::cout << "DONE" << std::endl;
    return 0;

}
