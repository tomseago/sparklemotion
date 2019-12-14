package baaahs.shaders

import baaahs.glsl.GlslPlugin
import baaahs.glsl.UvTranslatorForTest
import kotlinx.serialization.json.json
import kotlin.test.Test
import kotlin.test.expect

class GlslShaderTest {
    @Test
    fun testFindAdjustableValues() {
        val glslShader = GlslShader(
            """
            // SPARKLEMOTION GADGET: Slider {name: "Scale", initialValue: 5.0, minValue: 1.0, maxValue: 10.0}
            uniform float scale;
        """.trimIndent(), UvTranslatorForTest
        )
        expect(
            listOf(
                GlslShader.AdjustableValue(
                    "scale",
                    GlslPlugin.GadgetDataSourceProvider("Slider"),
                    GlslShader.AdjustableValue.Type.FLOAT,
                    json { "name" to "Scale"; "initialValue" to 5.0; "minValue" to 1.0; "maxValue" to 10.0 }
                )
            )
        ) { glslShader.adjustableValues.filter { it.varName == "scale" } }
    }

    @Test
    fun testFindPluginParams() {
        val glslShader = GlslShader(
            """
            // SPARKLEMOTION PLUGIN: SoundAnalysis {}
            uniform float lows;
        """.trimIndent(), UvTranslatorForTest
        )
        expect(
            listOf(
                GlslShader.AdjustableValue(
                    "lows",
                    GlslPlugin.PluginDataSourceProvider("SoundAnalysis"),
                    GlslShader.AdjustableValue.Type.FLOAT,
                    json { }
                )
            )
        ) { glslShader.adjustableValues.filter { it.varName == "lows" } }
    }
}