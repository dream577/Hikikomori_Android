package com.violet.libbasetools.util

/**
 * 扩展属性，为一个类生成一个唯一的字符串标志
 */
val Any.tag: String
    get() = this.javaClass.simpleName

/**
 * String 扩展函数，抽取字符串中的数字
 */
fun String?.extractDigit(): Long {
    if (this == null) {
        return 0
    }
    val num = this.replace("\\D+".toRegex(), "")

    if (num.isEmpty())
        return 0

    return num.toLong()
}

/**
 * 扩展函数: 查找list中所有符合条件的集合
 */
fun <T> Iterable<T>.findAll(predicate: (T) -> Boolean): List<T> {
    val result = mutableListOf<T>()
    this.forEach {
        if (predicate(it)) {
            result.add(it)
        }
    }
    return result
}
