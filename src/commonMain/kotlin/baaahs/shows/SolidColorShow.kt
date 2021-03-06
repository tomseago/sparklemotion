package baaahs.shows

import baaahs.Color
import baaahs.Model
import baaahs.Show
import baaahs.ShowRunner
import baaahs.gadgets.ColorPicker
import baaahs.gadgets.Slider
import baaahs.shaders.SolidShader

object SolidColorShow : Show("Solid Color") {
    override fun createRenderer(model: Model<*>, showRunner: ShowRunner): Renderer {
        val colorPicker = showRunner.getGadget("color", ColorPicker("Color"))
        val saturationPicker = showRunner.getGadget("sm_saturation", Slider("Saturation"))
        val brightnessPicker = showRunner.getGadget("sm_brightness", Slider("Brightness"))

        val shader = SolidShader()
        val shaderBuffers = showRunner.allSurfaces.map {
            showRunner.getShaderBuffer(it, shader).apply { color = Color.ORANGE }
        }

        val eyes = model.movingHeads.map { eye -> showRunner.getMovingHeadBuffer(eye) }

        return object : Renderer {
            override fun nextFrame() {
                val color = colorPicker.color
                shaderBuffers.forEach {
                    it.color = color.withSaturation(saturationPicker.value).withBrightness(brightnessPicker.value)
                }
                eyes.forEach { it.color = color }
            }
        }
    }
}