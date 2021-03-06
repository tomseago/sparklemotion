package baaahs.shows

import baaahs.*
import baaahs.shaders.PixelShader
import kotlin.random.Random

object PixelTweenShow : Show("PixelTweenShow") {
    override fun createRenderer(model: Model<*>, showRunner: ShowRunner): Renderer {
        val colorArray = arrayOf(
            Color.from("#FF8A47"),
            Color.from("#FC6170"),
            Color.from("#8CEEEE"),
            Color.from("#26BFBF"),
            Color.from("#FFD747")
        )

        return object : Renderer {
            val shaderBuffers = showRunner.allSurfaces.map { surface ->
                showRunner.getShaderBuffer(surface, PixelShader())
            }
            val fadeTimeMs = 1000

            override fun nextFrame() {
                val now = getTimeMillis().and(0xfffffff).toInt()
                shaderBuffers.forEachIndexed { i, buffer ->
                    val colorIndex = (now / fadeTimeMs + i) % colorArray.size
                    val startColor = colorArray[colorIndex]
                    val endColor = colorArray[(colorIndex + 1) % colorArray.size]

                    val colors = buffer.colors
                    colors.forEachIndexed { index, _ ->
                        if (Random.nextFloat() < .1) {
                            colors[index] = Color.WHITE
                        } else {
                            val tweenedColor =
                                startColor.fade(endColor, ((now + index) % fadeTimeMs) / fadeTimeMs.toFloat())
                            colors[index] = tweenedColor
                        }
                    }
                }
            }
        }
    }
}
