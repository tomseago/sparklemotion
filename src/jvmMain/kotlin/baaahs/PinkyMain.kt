package baaahs

import baaahs.dmx.DmxDevice
import baaahs.glsl.GlslBase
import baaahs.net.JvmNetwork
import baaahs.proto.Ports
import baaahs.shows.AllShows
import baaahs.sim.FakeDmxUniverse
import com.xenomachina.argparser.ArgParser
import com.xenomachina.argparser.default
import com.xenomachina.argparser.mainBody
import io.ktor.http.content.*
import io.ktor.routing.routing
import kotlinx.coroutines.GlobalScope
import kotlinx.coroutines.delay
import kotlinx.coroutines.launch
import java.io.File
import java.io.FileNotFoundException
import java.nio.file.FileSystems
import java.nio.file.Files
import java.nio.file.Path
import java.nio.file.Paths

fun main(args: Array<String>) {
    val config = mainBody(PinkyMain::class.simpleName) {
        ArgParser(args).parseInto(PinkyMain::Config)
    }

    PinkyMain(config).run()
}

class PinkyMain(val config: Config) {
    private val logger = Logger("PinkyMain")

    fun run() {
        GlslBase.manager // Need to wake up OpenGL on the main thread.

        val model = Pluggables.loadModel(config.modelName)

        val resource = Pinky::class.java.classLoader.getResource("baaahs")!!
        val useResources: Boolean
        val jsResDir = if (resource.protocol == "jar") {
            useResources = true
            val uri = resource.toURI()!!
            FileSystems.newFileSystem(uri, mapOf("create" to "true"))
            Paths.get(uri).parent.resolve("htdocs")
        } else {
            useResources = false
            val classPathBaseDir = Paths.get(resource.file).parent
            classPathBaseDir.parent.parent.parent.parent.parent
                .resolve("build/processedResources/js/main")
        }

        testForIndexDotHtml(jsResDir)

        val network = JvmNetwork()
        val dataDir = File(System.getProperty("user.home")).toPath().resolve("sparklemotion/data")
//        Files.createDirectories(dataDir)

        val fwDir = File(System.getProperty("user.home")).toPath().resolve("sparklemotion/fw")

        val fs = RealFs(dataDir)

        val dmxUniverse = findDmxUniverse()

        val beatLinkBeatSource = BeatLinkBeatSource(SystemClock())
        beatLinkBeatSource.start()

        val fwUrlBase = "http://${network.link().myAddress.address.hostAddress}:${Ports.PINKY_UI_TCP}/fw"
        val daddy = DirectoryDaddy(RealFs(fwDir), fwUrlBase)
        val shows = AllShows.allShows.filter { config.showName == null || config.showName == it.name }

        val display = object : StubPinkyDisplay() {
            override fun listShows(shows: List<Show>) {
                println("shows = $shows")
            }

            override var selectedShow: Show? = null
                set(value) {
                    field = value; println("selectedShow: $value")
                }

            override var showFrameMs: Int = 0
        }

        val pinky = Pinky(
            model, shows, network, dmxUniverse, beatLinkBeatSource, SystemClock(),
            fs, daddy, display, JvmSoundAnalyzer(),
            prerenderPixels = true,
            switchShowAfterIdleSeconds = config.switchShowAfter,
            adjustShowAfterIdleSeconds = config.adjustShowAfter
        )

        val ktor = (pinky.httpServer as JvmNetwork.RealLink.KtorHttpServer)
        ktor.application.routing {
            static {
                if (useResources) {
                    resources("htdocs")
                    defaultResource("htdocs/ui-index.html")
                } else {
                    files(jsResDir.toFile())
                    default(jsResDir.resolve("ui-index.html").toFile())
                }
            }

            static("fw") {
                files(fwDir.toFile())
            }
        }

        GlobalScope.launch {
            pinky.run()
        }

        doRunBlocking {
            delay(200000L)
        }
    }

    private fun testForIndexDotHtml(jsResDir: Path) {
        val indexHtml = jsResDir.resolve("index.html")
        if (!Files.exists(indexHtml)) {
            throw FileNotFoundException("$indexHtml doesn't exist and it really probably should!")
        }
    }

    private fun findDmxUniverse(): Dmx.Universe {
        val dmxDevices = try {
            DmxDevice.listDevices()
        } catch (e: UnsatisfiedLinkError) {
            logger.warn { "DMX driver not found, DMX will be disabled." }
            e.printStackTrace()
            return FakeDmxUniverse()
        }

        if (dmxDevices.isNotEmpty()) {
            if (dmxDevices.size > 1) {
                logger.warn { "Multiple DMX USB devices found, using ${dmxDevices.first()}." }
            }

            return dmxDevices.first()
        }

        logger.warn { "No DMX USB devices found, DMX will be disabled." }
        return FakeDmxUniverse()
    }

    class Config(parser: ArgParser) {
        val modelName by parser.storing("model").default(Pluggables.defaultModel)

        val showName by parser.storing("show").default<String?>(null)

        val switchShowAfter by parser.storing(
            "Switch show after no input for x seconds",
            transform = { if (isNullOrEmpty()) null else toInt() })
            .default<Int?>(600)

        val adjustShowAfter by parser.storing(
            "Start adjusting show inputs after no input for x seconds",
            transform = { if (isNullOrEmpty()) null else toInt() })
            .default<Int?>(null)
    }
}
