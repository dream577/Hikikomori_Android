package com.violet.libmedia.util;

import java.util.concurrent.LinkedBlockingQueue;
import java.util.concurrent.TimeUnit;

import kotlin.jvm.functions.Function1;

public class RecycledPool<T> {
    private final int capacity;
    private final LinkedBlockingQueue<Element<T>> queue;

    public RecycledPool(int capacity, Function1<Integer, T> func) {
        this.capacity = capacity;
        this.queue = new LinkedBlockingQueue<>(capacity);
        for (int i = 0; i < capacity; i++) {
            T t = func.invoke(i);
            Element<T> element = new Element<>(this, t);
            queue.offer(element);
        }
    }

    public RecycledPool(int capacity) {
        this.capacity = capacity;
        this.queue = new LinkedBlockingQueue<>(capacity);
    }

    public void initRecycledPool(Function1<Integer, T> func) {
        for (int i = 0; i < capacity; i++) {
            T t = func.invoke(i);
            Element<T> element = new Element<>(this, t);
            queue.offer(element);
        }
    }

    public Element<T> take(long timeout, TimeUnit unit) throws InterruptedException {
        return queue.poll(timeout, unit);
    }

    public Element<T> take() throws InterruptedException {
        return queue.take();
    }


    public void clear() {
        queue.clear();
    }

    public static class Element<E> {
        private final E value;
        private final RecycledPool<E> mRecycledPool;

        private Element(RecycledPool<E> pool, E e) {
            this.mRecycledPool = pool;
            this.value = e;
        }

        public E getValue() {
            return value;
        }

        public void recycle() {
            mRecycledPool.queue.offer(this);
        }
    }
}
