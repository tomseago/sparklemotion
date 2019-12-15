package baaahs.plugins

import baaahs.BeatSource
import baaahs.SoundAnalyzer
import baaahs.glsl.GlslPlugin
import baaahs.glsl.Program
import baaahs.glsl.Uniform
import baaahs.glsl.check
import com.danielgergely.kgl.*
import kotlinx.serialization.json.JsonObject

class BeatPlugin(val beatSource: BeatSource) : GlslPlugin {
    override val name: String = "Beat"

    override fun createDataSource(config: JsonObject): GlslPlugin.DataSource {
        return object : GlslPlugin.DataSource {
            override fun getValue(): Any = beatSource.getBeatData()
        }
    }

    override fun forProgram(gl: Kgl, program: Program): GlslPlugin.ProgramContext {
        TODO("not implemented") //To change body of created functions use File | Settings | File Templates.
    }
}