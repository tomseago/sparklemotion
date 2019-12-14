package baaahs.glsl

import baaahs.geom.Vector3F
import baaahs.io.ByteArrayWriter

object UvTranslatorForTest : UvTranslator(Id.PANEL_SPACE_UV_TRANSLATOR) {
    override fun serializeConfig(writer: ByteArrayWriter) = TODO("not implemented")

    override fun forPixels(pixelLocations: List<Vector3F?>) = object :
        SurfaceUvTranslator {
        override val pixelCount = pixelLocations.count()
        override fun getUV(pixelIndex: Int): Pair<Float, Float> = pixelLocations[pixelIndex]!!.let { it.x to it.y }
    }
}