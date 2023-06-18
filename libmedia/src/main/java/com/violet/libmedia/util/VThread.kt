package com.violet.libmedia.util

import java.util.concurrent.LinkedBlockingQueue
import java.util.concurrent.TimeUnit

open class VThread(name: String) : Thread(name) {
    private val queue = LinkedBlockingQueue<Int>()
    private val stopFlag = -10000

    @Volatile
    private var stop = false

    private var loopMsg = -1  // 循环消息

    final override fun run() {
        super.run()

        try {
            while (!stop) {
                if (queue.isEmpty() && loopMsg != -1) {   // 消息队列为空且已指定循环事件
                    handleMessage(loopMsg)
                } else {
                    val msg = queue.poll(100, TimeUnit.MILLISECONDS) ?: continue
                    if (msg == stopFlag) { // 收到停止事件
                        stop = true
                        queue.clear()
                        return
                    }
                    handleMessage(msg)
                }
            }
        } catch (e: Exception) {
            e.printStackTrace()
        }
    }

    protected fun startLoop(message: Int) {
        loopMsg = message
        putMessage(message)
    }

    protected fun quit() {
        putMessage(stopFlag)
    }

    protected fun putMessage(message: Int) {
        if (stop) return
        queue.put(message)
    }

    protected open fun handleMessage(msg: Int) {

    }
}