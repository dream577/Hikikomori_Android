package com.violet.libbasetools.model

class Event<T>(data: T) {
    private var data: T
    private var handled: Boolean

    init {
        this.data = data
        this.handled = false
    }

    fun getDataIfNotHandled(): T? {
        return if (handled) {
            null
        } else {
            handled = true
            data
        }
    }

    fun isHandled(): Boolean {
        return handled
    }
}