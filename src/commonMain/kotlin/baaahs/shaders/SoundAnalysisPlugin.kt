package baaahs.shaders

import baaahs.SoundAnalyzer
import baaahs.glsl.*
import baaahs.glsl.Program
import com.danielgergely.kgl.*

class SoundAnalysisPlugin(val soundAnalyzer: SoundAnalyzer, val historySize: Int = 300) : GlslPlugin {
    private var textureBuffer = FloatArray(0)
    private var textureGlBuffer = FloatBuffer(0)

    init {
        soundAnalyzer.listen(object : SoundAnalyzer.AnalysisListener {
            override fun onSample(analysis: SoundAnalyzer.Analysis) {
                val analysisBufferSize = soundAnalyzer.frequencies.size
                val expectedBufferSize = analysisBufferSize * historySize
                if (textureBuffer.size != expectedBufferSize) {
                    textureBuffer = FloatArray(expectedBufferSize)
                    textureGlBuffer = FloatBuffer(expectedBufferSize)
                }

                // Shift historical data down one row.
                textureBuffer.copyInto(textureBuffer, analysisBufferSize, 0, expectedBufferSize - analysisBufferSize)

                // Copy this sample's data into the buffer.
                analysis.magnitudes.forEachIndexed { index, magitude ->
                    textureBuffer[index] = magitude * analysisBufferSize
                }
            }
        })
    }

    override fun forRenderer(renderer: GlslRenderer): GlslPlugin.RendererPlugin {
        return RendererPlugin(renderer)
    }

    inner class RendererPlugin(val renderer: GlslRenderer) : GlslPlugin.RendererPlugin {
        override val glslPreamble: String = "uniform sampler2D sm_soundAnalysis;"

        private val gl = renderer.gl
        private val texture = gl.check { gl.createTexture() }
        private val textureId = renderer.getTextureId()
        private var soundAnalysisUniform: Uniform? = null

        override fun afterCompile(program: Program) {
            soundAnalysisUniform = gl.check { Uniform.find(program, "sm_soundAnalysis") }
        }

        override fun beforeRender() {
            val analysisBufferSize = soundAnalyzer.frequencies.size
            val expectedBufferSize = analysisBufferSize * historySize

            val uniform = soundAnalysisUniform
            if (uniform == null || textureBuffer.size != expectedBufferSize) return

            textureGlBuffer.position = 0
            textureGlBuffer.put(textureBuffer)

            gl.check { gl.activeTexture(GL_TEXTURE0 + textureId) }
            gl.check { gl.bindTexture(GL_TEXTURE_2D, texture) }
            gl.check { gl.texParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST) }
            gl.check { gl.texParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST) }
            gl.check {
                gl.texImage2D(
                    GL_TEXTURE_2D, 0,
                    GL_R32F, soundAnalyzer.frequencies.size, historySize, 0,
                    GL_RED, GL_FLOAT, textureGlBuffer
                )
            }
            uniform.set(textureId)
        }

        override fun release() {
            gl.check { gl.deleteTexture(texture) }
        }
    }
}