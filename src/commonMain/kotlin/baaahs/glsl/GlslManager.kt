package baaahs.glsl

import com.danielgergely.kgl.Kgl

abstract class GlslManager(private val glslVersion: String) {
    protected abstract val kgl: Kgl
    private val contextSwitcher = object : GlslRenderer.ContextSwitcher {
        override fun <T> inContext(fn: () -> T): T = runInContext(fn)
    }

    abstract fun <T> runInContext(fn: () -> T): T

    fun createProgram(fragShader: String): Program {
        return runInContext {
            Program(kgl, fragShader, glslVersion, GlslBase.plugins)
        }
    }

    fun createRenderer(program: Program): GlslRenderer {
        return runInContext {
            GlslRenderer(kgl, contextSwitcher, program)
        }
    }
}