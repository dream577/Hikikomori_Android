package com.violet.libnetwork.interceptors

import com.google.gson.Gson
import com.google.gson.JsonElement
import com.google.gson.JsonSyntaxException

import com.violet.libbasetools.KLog
import com.violet.libnetwork.model.Constant
import okhttp3.Interceptor
import okhttp3.Response
import okio.Buffer

class HttpPrettyLogInterceptor : Interceptor {
    private val DEFAULT_TAG = "HTTP_LOG"
    private val gson: Gson = Gson()

    override fun intercept(chain: Interceptor.Chain): Response {
        val originalRequest = chain.request()
        val connection = chain.connection()
        val uniqueKey = originalRequest.hashCode()

        val requestMsg = StringBuilder("")
        val responseMsg = StringBuilder("")

        val requestHeaders = originalRequest.headers
        val requestBody = originalRequest.body

        // 请求字符串构建开始
        requestMsg.append("Request[$uniqueKey] start: ").append(Constant.NEW_LINE)
        requestMsg.append(originalRequest.method).append(Constant.SPACE)
            .append(originalRequest.url.toUri().path)
            .append(Constant.NEW_LINE)
        requestMsg.append(requestHeaders.toString()).append(Constant.THE_END)

        if (requestBody != null) {
            val buffer = Buffer()
            requestBody.writeTo(buffer)
            val body = buffer.readUtf8()
            try {
                val je = gson.fromJson(body, JsonElement::class.java)
                val jsonString = gson.toJson(je)
                requestMsg.append(jsonString).append(Constant.NEW_LINE)
            } catch (e: JsonSyntaxException) {
                requestMsg.append(Constant.NEW_LINE)
                e.printStackTrace()
            }
        } else {
            requestMsg.append(Constant.NEW_LINE)
        }
        KLog.d(DEFAULT_TAG, requestMsg)
        // 请求字符串构建结束

        val response = chain.proceed(originalRequest)

        val responseHeaders = response.headers
        val responseBody = response.body
        // 响应字符串构建开始
        val reqSentTime = response.sentRequestAtMillis
        val respReceivedTime = response.receivedResponseAtMillis
        responseMsg.append("send request at: ").append(reqSentTime).append(Constant.NEW_LINE)
        responseMsg.append("receive response at: ").append(respReceivedTime)
            .append(Constant.NEW_LINE)
        responseMsg.append("cost time: ").append(respReceivedTime - reqSentTime)
            .append(Constant.THE_END)
        responseMsg.append("Response[$uniqueKey] start:").append(Constant.NEW_LINE)
        responseMsg.append(connection?.protocol()?.toString() ?: "").append(Constant.SPACE)
            .append(response.code).append(Constant.SPACE)
            .append(response.message).append(Constant.NEW_LINE)
        responseMsg.append(responseHeaders.toString()).append(Constant.THE_END)

        if (responseBody != null) {
            val buffer = responseBody.source().buffer.clone()
            val body = buffer.readUtf8()
            try {
                val je = gson.fromJson(body, JsonElement::class.java)
                val jsonString = gson.toJson(je)
                requestMsg.append(jsonString).append(Constant.NEW_LINE)
            } catch (e: JsonSyntaxException) {
                requestMsg.append(Constant.NEW_LINE)
                e.printStackTrace()
            }
        } else {
            requestMsg.append(Constant.NEW_LINE)
        }
        KLog.d(DEFAULT_TAG, responseMsg)
        // 响应字符串构建结束

        return response
    }
}