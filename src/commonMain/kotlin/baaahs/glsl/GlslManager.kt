package baaahs.glsl

import baaahs.shaders.GlslShader

interface GlslManager {
    fun createRenderer(
        fragShader: String,
        params: List<GlslShader.Param>,
        plugins: List<GlslPlugin> = GlslBase.plugins
    ): GlslRenderer
}