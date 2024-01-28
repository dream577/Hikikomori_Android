//
// Created by bronyna on 2023/7/15.
//

#ifndef HIKIKOMORI_LINKEDBLOCKINGQUEUE_H
#define HIKIKOMORI_LINKEDBLOCKINGQUEUE_H

#include <mutex>
#include <condition_variable>
#include <atomic>
#include <memory>

using namespace std;

template<typename E>
class Node {
    template<class T>
    friend
    class LinkedBlockingQueue;

private:
    shared_ptr<E> data;
    shared_ptr<Node<E>> next;

public:
    Node() {
    }

    ~Node() {
    }
};

template<typename E>
class LinkedBlockingQueue {
private:
    int capability;
    bool abort;
    atomic_int count;

    mutex mux;
    condition_variable cond;

    shared_ptr<Node<E>> head;
    shared_ptr<Node<E>> tail;

    void _enqueue(shared_ptr<E> data) {
        shared_ptr<Node<E>> node = make_shared<Node<E>>();
        node->data = data;
        tail = tail->next = node;
        ++count;
    }

    shared_ptr<E> _dequeue() {
        shared_ptr<Node<E>> first = head->next;
        head = first;
        --count;
        return first->data;
    }

    void _clear() {
        while (count != 0) {
            _dequeue();
        }
        head = tail;
    }

public:

    LinkedBlockingQueue(int cap) {
        this->capability = cap;
        this->abort = false;
        atomic_init(&count, 0);
        head = tail = make_shared<Node<E>>();
    }

    LinkedBlockingQueue(int cap, std::function<shared_ptr<E>(int)> const &fun) {
        this->capability = cap;
        this->abort = false;
        atomic_init(&count, 0);
        head = tail = make_shared<Node<E>>();
        for (int i = 0; i < capability; i++) {
            auto data = fun(i);
            put(data);
        }
    }

    bool put(shared_ptr<E> data) {
        unique_lock<mutex> lock(mux);
        if (count == capability) return false;
        _enqueue(data);
        lock.unlock();
        cond.notify_all();
        return true;
    }

    bool offer(shared_ptr<E> data) {
        unique_lock<mutex> lock(mux);
        while (count == capability && !abort) {
            cond.wait(lock);
        }
        if (!abort) {
            _enqueue(data);
            lock.unlock();
            cond.notify_all();
            return true;
        } else {
            return false;
        }
    }

    shared_ptr<E> take() {
        unique_lock<mutex> lock(mux);
        if (count == 0) {
            return shared_ptr<E>();
        }
        shared_ptr<E> data = _dequeue();
        lock.unlock();
        cond.notify_all();
        return data;
    }

    shared_ptr<E> poll() {
        unique_lock<mutex> lock(mux);
        while (count == 0 && !abort) {
            cond.wait(lock);
        }
        if (count == 0) {
            return shared_ptr<E>();
        }
        shared_ptr<E> data = _dequeue();
        lock.unlock();
        cond.notify_all();
        return data;
    }

    int size() {
        unique_lock<mutex> lock(mux);
        return count;
    }

    void overrule() {
        unique_lock<mutex> lock(mux);
        abort = true;
        lock.unlock();
        cond.notify_all();
    }

    void clear() {
        unique_lock<mutex> lock(mux);
        _clear();
        lock.unlock();
        cond.notify_all();
    }

    ~LinkedBlockingQueue() {
        unique_lock<mutex> lock(mux);
        abort = true;
        _clear();
        lock.unlock();
        cond.notify_all();
    }
};

template<class E>
class RecyclerPool {

};


#endif //HIKIKOMORI_LINKEDBLOCKINGQUEUE_H
