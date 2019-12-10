package baaahs.glsl

import com.danielgergely.kgl.Kgl
import com.danielgergely.kgl.KglJs
import org.w3c.dom.HTMLCanvasElement
import kotlin.browser.document
import kotlin.browser.window

actual object GlslBase {
    actual val plugins: MutableList<GlslPlugin> = mutableListOf()
    actual val manager: GlslManager by lazy { JsGlslManager() }

    class JsGlslManager : GlslManager("300 es") {
        override val kgl = createContext()

        override fun <T> runInContext(fn: () -> T): T = fn()

        private fun createContext(): Kgl {
            val canvas = document.createElement("canvas") as HTMLCanvasElement
            val gl = canvas.getContext("webgl2")
            if (gl == null) {
                window.alert(
                    "Running GLSL shows on iOS requires WebGL 2.0.\n" +
                            "\n" +
                            "Go to Settings → Safari → Advanced → Experimental Features and enable WebGL 2.0."
                )
                throw Exception("WebGL 2 not supported")
            }
            return KglJs(gl.asDynamic())
        }
    }
}
