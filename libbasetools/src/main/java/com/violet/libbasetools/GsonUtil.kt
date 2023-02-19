package com.violet.libbasetools

import com.google.gson.Gson
import com.google.gson.JsonSyntaxException

private val gson: Gson = Gson().newBuilder()
    .setPrettyPrinting()                     // 设置格式化打印
    .excludeFieldsWithoutExposeAnnotation()  //	允许属性执行Expose注解,默认是不允许的. 当Java bean使用了Expose注解需要调用该方法才会生效
    .serializeNulls()                        // 对null进行序列化，默认是不允许的
    .setVersion(1.0)                         // @Since @Until
    .create()

fun <T> toJson(data: T): String {
    return gson.toJson(data)
}

fun <T> fromJson(json: String, clazz: Class<T>): T {
    var data: T = clazz.newInstance()
    try {
        data = gson.fromJson(json, clazz)
    } catch (e: JsonSyntaxException) {
        e.printStackTrace()
    }
    return data
}