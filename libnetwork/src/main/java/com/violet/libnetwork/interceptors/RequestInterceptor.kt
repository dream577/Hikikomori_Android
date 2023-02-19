package com.violet.libnetwork.interceptors

import com.violet.libnetwork.model.Constant
import okhttp3.Interceptor
import okhttp3.Request
import okhttp3.Response

class RequestInterceptor : Interceptor {
    override fun intercept(chain: Interceptor.Chain): Response {
        val request: Request = chain.request()
        val headers = request.headers
        val host: String = chain.request().url.toUrl().getHost()
        val newBuilder = headers.newBuilder()
            .add(Constant.AUTHENTICATION, "Moon App")
            .add(Constant.HOST, host)
        request.headers.newBuilder().addAll(newBuilder.build())
        return chain.proceed(request)
    }
}