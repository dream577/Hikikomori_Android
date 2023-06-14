package com.violet.libmedia.util

import java.util.concurrent.LinkedTransferQueue
import java.util.concurrent.TimeUnit

open class VThread(name: String) : Thread(name) {
    private val queue = LinkedTransferQueue<Int>()
    private val stop = false
    private val stopFlag = -10000
    private var loopMsg = -1

    override fun run() {
        super.run()

        while (!stop) {
            if (queue.isEmpty() && loopMsg != -1) {
                handleMessage(loopMsg)
            } else {
                val msg = queue.poll(10, TimeUnit.MILLISECONDS) ?: continue
                if (msg == stopFlag) {
                    break
                }
                handleMessage(msg)
            }
        }
    }

    fun startLoop(message: Int) {
        loopMsg = message
        putMessage(message)
    }

    fun quit() {
        putMessage(stopFlag)
    }

    fun putMessage(message: Int) {
        queue.put(message)
    }

    open fun handleMessage(msg: Int) {

    }
}