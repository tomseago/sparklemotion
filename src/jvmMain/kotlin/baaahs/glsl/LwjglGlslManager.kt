package baaahs.glsl

import com.danielgergely.kgl.KglLwjgl
import org.lwjgl.glfw.GLFW
import org.lwjgl.opengl.GLCapabilities

class LwjglGlslManager : GlslManager("330 core") {
    private val window: Long

    /**
     * This is initialization stuff that's required on the main thread.
     */
    init {
//        GLFW.glfwSetErrorCallback(GLFWErrorCallback.createPrint(System.err))
        check(GLFW.glfwInit()) { "Unable to initialize GLFW" }

        GLFW.glfwDefaultWindowHints()
        GLFW.glfwWindowHint(GLFW.GLFW_CONTEXT_VERSION_MAJOR, 3)
        GLFW.glfwWindowHint(GLFW.GLFW_CONTEXT_VERSION_MINOR, 2)
        GLFW.glfwWindowHint(GLFW.GLFW_OPENGL_PROFILE, GLFW.GLFW_OPENGL_CORE_PROFILE)
        GLFW.glfwWindowHint(GLFW.GLFW_OPENGL_FORWARD_COMPAT, GLFW.GLFW_TRUE)
        GLFW.glfwWindowHint(GLFW.GLFW_VISIBLE, GLFW.GLFW_FALSE)
        GLFW.glfwWindowHint(GLFW.GLFW_RESIZABLE, GLFW.GLFW_FALSE)
        GLFW.glfwWindowHint(GLFW.GLFW_SAMPLES, 8)
        GLFW.glfwWindowHint(GLFW.GLFW_CONTEXT_RELEASE_BEHAVIOR, GLFW.GLFW_RELEASE_BEHAVIOR_NONE)
        GLFW.glfwWindowHint(GLFW.GLFW_OPENGL_DEBUG_CONTEXT, GLFW.GLFW_TRUE)

        window = GLFW.glfwCreateWindow(300, 300, "Hello shaders!", 0, 0)
        if (window == 0L)
            throw RuntimeException("Failed to create the GLFW window")

        GLFW.glfwPollEvents() // Get the event loop warmed up.
    }

    override val kgl = KglLwjgl()

    override fun <T> runInContext(fn: () -> T): T {
        GLFW.glfwMakeContextCurrent(window)
        glCapabilities.get() // because it's expensive and only has to happen once per thread

        try {
            return fn()
        } finally {
            GLFW.glfwMakeContextCurrent(0)
        }
    }

    companion object {
        val glCapabilities: ThreadLocal<GLCapabilities> =
            ThreadLocal.withInitial { org.lwjgl.opengl.GL.createCapabilities() }
    }
}