package com.violet.hikikomori.view.util

import com.violet.hikikomori.R

object AnimUtil {
    const val DIRECTION_ENTER_FROM_LEFT = 0
    const val DIRECTION_ENTER_FROM_RIGHT = 1
    const val DIRECTION_ENTER_FROM_TOP = 2
    const val DIRECTION_ENTER_FROM_BOTTOM = 3
    const val DIRECTION_OUT_FROM_LEFT = 4
    const val DIRECTION_OUT_FROM_RIGHT = 5
    const val DIRECTION_OUT_FROM_TOP = 6
    const val DIRECTION_OUT_FROM_BOTTOM = 7

    fun getAnimation(direction: Int): Int {
        return when (direction) {
            DIRECTION_ENTER_FROM_LEFT -> R.anim.slide_in_from_left
            DIRECTION_ENTER_FROM_RIGHT -> R.anim.slide_in_from_right
            DIRECTION_ENTER_FROM_TOP -> R.anim.slide_in_from_top
            DIRECTION_ENTER_FROM_BOTTOM -> R.anim.slide_in_from_bottom
            DIRECTION_OUT_FROM_LEFT -> R.anim.slide_out_from_left
            DIRECTION_OUT_FROM_RIGHT -> R.anim.slide_out_from_right
            DIRECTION_OUT_FROM_BOTTOM -> R.anim.slide_out_from_bottom
            DIRECTION_OUT_FROM_TOP -> R.anim.slide_out_from_top
            else -> R.anim.slide_in_from_left
        }
    }
}