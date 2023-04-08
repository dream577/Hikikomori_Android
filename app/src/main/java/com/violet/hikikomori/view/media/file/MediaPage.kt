package com.violet.hikikomori.view.media.file

import com.violet.hikikomori.view.base.BaseFragment
import com.violet.libbasetools.util.tag

class MediaPage(
    val tag: String,
    val fragment: BaseFragment,
    val title: String
) {
    constructor(fragment: BaseFragment, title: String) : this(fragment.javaClass.tag, fragment, title)
}