package com.violet.libbasetools.util

import com.example.libbasetools.BuildConfig
import com.orhanobut.logger.FormatStrategy
import com.orhanobut.logger.PrettyFormatStrategy
import com.orhanobut.logger.AndroidLogAdapter
import com.orhanobut.logger.Logger

object KLog {
    init {
        val formatStrategy: FormatStrategy = PrettyFormatStrategy.newBuilder()
            .showThreadInfo(true)
            .methodCount(1)          // (Optional) How many method line to show. Default 2
            .methodOffset(7)        // (Optional) Hides internal method calls up to offset. Default 5
            // .logStrategy(customLog)  // (Optional) Changes the log strategy to print out. Default LogCat
            .tag("PRETTY_MOON_LOG")     // (Optional) Global tag for every log. Default PRETTY_LOGGER
            .build()
        Logger.addLogAdapter(AndroidLogAdapter(formatStrategy))
    }

    @JvmStatic
    fun v(msg: Any) {
        if (BuildConfig.LOG_DEBUG) Logger.v(msg.toString())
    }

    @JvmStatic
    fun v(tag: String?, msg: Any) {
        if (BuildConfig.LOG_DEBUG) Logger.log(Logger.VERBOSE, tag, msg.toString(), null)
    }

    @JvmStatic
    fun v(tag: String?, msg: Any, throwable: Throwable?) {
        if (BuildConfig.LOG_DEBUG) Logger.log(Logger.VERBOSE, tag, msg.toString(), null)
    }

    @JvmStatic
    fun d(o: Any?) {
        if (BuildConfig.LOG_DEBUG) Logger.d(o)
    }

    @JvmStatic
    fun d(msg: String?) {
        if (BuildConfig.LOG_DEBUG) Logger.d(msg)
    }

    @JvmStatic
    fun d(tag: String?, msg: Any) {
        if (BuildConfig.LOG_DEBUG) Logger.log(Logger.DEBUG, tag, msg.toString(), null)
    }

    @JvmStatic
    fun d(tag: String?, msg: Any, throwable: Throwable?) {
        if (BuildConfig.LOG_DEBUG) Logger.log(Logger.DEBUG, tag, msg.toString(), throwable)
    }

    @JvmStatic
    fun i(msg: String?) {
        if (BuildConfig.LOG_DEBUG) Logger.i(msg!!)
    }

    @JvmStatic
    fun i(tag: String?, msg: Any) {
        if (BuildConfig.LOG_DEBUG) Logger.log(Logger.INFO, tag, msg.toString(), null)
    }

    @JvmStatic
    fun i(tag: String?, msg: Any, throwable: Throwable?) {
        if (BuildConfig.LOG_DEBUG) Logger.log(Logger.INFO, tag, msg.toString(), throwable)
    }

    @JvmStatic
    fun w(msg: String?) {
        if (BuildConfig.LOG_DEBUG) Logger.w(msg!!)
    }

    @JvmStatic
    fun w(tag: String?, msg: Any) {
        if (BuildConfig.LOG_DEBUG) Logger.log(Logger.WARN, tag, msg.toString(), null)
    }

    @JvmStatic
    fun w(tag: String?, msg: Any, throwable: Throwable?) {
        if (BuildConfig.LOG_DEBUG) Logger.log(Logger.WARN, tag, msg.toString(), throwable)
    }

    @JvmStatic
    fun e(msg: String?) {
        if (BuildConfig.LOG_DEBUG) Logger.e(msg!!)
    }

    @JvmStatic
    fun e(tag: String?, msg: Any) {
        if (BuildConfig.LOG_DEBUG) Logger.log(Logger.ERROR, tag, msg.toString(), null)
    }

    @JvmStatic
    fun e(tag: String?, msg: Any, throwable: Throwable?) {
        if (BuildConfig.LOG_DEBUG) Logger.log(Logger.ERROR, tag, msg.toString(), throwable)
    }

    @JvmStatic
    fun wtf(msg: String?) {
        if (BuildConfig.LOG_DEBUG) Logger.wtf(msg!!)
    }

    @JvmStatic
    fun wtf(tag: String?, msg: Any) {
        if (BuildConfig.LOG_DEBUG) Logger.log(Logger.ASSERT, tag, msg.toString(), null)
    }

    @JvmStatic
    fun wtf(tag: String?, msg: Any, throwable: Throwable?) {
        if (BuildConfig.LOG_DEBUG) Logger.log(Logger.ASSERT, tag, msg.toString(), throwable)
    }

    @JvmStatic
    fun json(json: String?) {
        if (BuildConfig.LOG_DEBUG) Logger.json(json)
    }

    @JvmStatic
    fun xml(xml: String?) {
        if (BuildConfig.LOG_DEBUG) Logger.xml(xml)
    }
}