package baaahs

import baaahs.net.Network
import baaahs.net.TestNetwork
import baaahs.proto.BrainHelloMessage
import baaahs.shaders.SolidShader
import baaahs.sim.FakeDmxUniverse
import kotlinx.coroutines.InternalCoroutinesApi
import kotlin.test.BeforeTest
import kotlin.test.Test
import kotlin.test.expect

@InternalCoroutinesApi
class PinkyTest {
    private lateinit var network: TestNetwork
    private lateinit var clientAddress: Network.Address

    private val panel17 = SheepModel.Panel("17")
    private val model = SheepModel().apply { panels = listOf(panel17) }
    private lateinit var testShow1: TestShow1
    private lateinit var pinky: Pinky
    private lateinit var pinkyLink: TestNetwork.Link

    @BeforeTest
    fun setUp() {
        network = TestNetwork(1_000_000)
        clientAddress = TestNetwork.Address("client")
        testShow1 = TestShow1()
        pinky = Pinky(model, listOf(testShow1), network, FakeDmxUniverse(), StubBeatSource(), StubPinkyDisplay(),
            FakeClock())
        pinkyLink = network.links.only()
    }

    @Test
    fun whenUnmappedBrainUnknownToPinkyComesOnline_showShouldBeNotified() {
        pinky.receive(clientAddress, BrainHelloMessage("brain1", null).toBytes())
        pinky.updateSurfaces()
        pinky.drawNextFrame()

        val show = testShow1.createdShows.only()
        expect(1) { show.shaderBuffers.size }
        expect(0) { pinkyLink.packetsToSend.size }
    }

    @Test
    fun whenUnmappedBrainKnownToPinkyComesOnline_showShouldBeNotifiedAndBrainShouldReceiveMapping() {
        pinky.providePanelMapping(BrainId("brain1"), panel17)

        pinky.receive(clientAddress, BrainHelloMessage("brain1", null).toBytes())
        pinky.updateSurfaces()
        pinky.drawNextFrame()

        val show = testShow1.createdShows.only()
        expect(1) { show.shaderBuffers.size }
        expect(1) { pinkyLink.packetsToSend.size }
    }

    @Test
    fun whenMappedBrainComesOnline_showShouldBeNotified() {
        pinky.providePanelMapping(BrainId("brain1"), panel17)

        pinky.receive(clientAddress, BrainHelloMessage("brain1", panel17.name).toBytes())
        pinky.updateSurfaces()
        pinky.drawNextFrame()

        val show = testShow1.createdShows.only()
        expect(1) { show.shaderBuffers.size }
        expect(0) { pinkyLink.packetsToSend.size }
    }

    @Test
    fun asBrainsComeOnlineAndAreMapped_showShouldBeNotified() {
        pinky.providePanelMapping(BrainId("brain1"), panel17)

        pinky.receive(clientAddress, BrainHelloMessage("brain1", null).toBytes())
        pinky.updateSurfaces()
        pinky.drawNextFrame()
        pinky.drawNextFrame()

        val show = testShow1.createdShows.only()
        expect(1) { show.shaderBuffers.size }
        expect(true) { show.shaderBuffers.keys.only() is Pinky.UnknownSurface }

        pinky.receive(clientAddress, BrainHelloMessage("brain1", panel17.name).toBytes())
        pinky.updateSurfaces()
        pinky.drawNextFrame()
        pinky.drawNextFrame()
        expect(1) { show.shaderBuffers.size }
        val surface = show.shaderBuffers.keys.only()
        expect(true) { surface is SheepModel.Panel }
        expect(panel17.name) { (surface as SheepModel.Panel).name }
    }

    @Test
    fun whenBrainHelloRaceCondition_asBrainsComeOnline_showShouldBeNotified() {
        pinky.receive(clientAddress, BrainHelloMessage("brain1", null).toBytes())
        pinky.updateSurfaces()
        pinky.drawNextFrame()
        pinky.drawNextFrame()
        val show = testShow1.createdShows.only()

        // Remap to 17L...
        pinky.receive(clientAddress, BrainHelloMessage("brain1", panel17.name).toBytes())
        // ... but a packet also made it through identifying brain1 as unmapped.
        pinky.receive(clientAddress, BrainHelloMessage("brain1", null).toBytes())
        pinky.updateSurfaces()
        pinky.drawNextFrame()
        pinky.drawNextFrame()

        // Pinky should have sent out another BrainMappingMessage message; todo: verify that!

        pinky.receive(clientAddress, BrainHelloMessage("brain1", panel17.name).toBytes())
        pinky.updateSurfaces()
        pinky.drawNextFrame()
        pinky.drawNextFrame()

        expect(1) { show.shaderBuffers.size }
        expect(true) { show.shaderBuffers.keys.only() == panel17 }

        pinky.receive(clientAddress, BrainHelloMessage("brain1", panel17.name).toBytes())
        pinky.updateSurfaces()
        pinky.drawNextFrame()
        pinky.drawNextFrame()
        expect(1) { show.shaderBuffers.size }
        val surface = show.shaderBuffers.keys.only()
        expect(true) { surface is SheepModel.Panel }
        expect(panel17.name) { (surface as SheepModel.Panel).name }
    }

    class TestShow1(var supportsSurfaceChange: Boolean = true) : Show("TestShow1") {
        val createdShows = mutableListOf<ShowRenderer>()
        val solidShader = SolidShader()

        override fun createRenderer(sheepModel: SheepModel, showRunner: ShowRunner): Renderer {
            return ShowRenderer(showRunner).also { createdShows.add(it) }
        }

        inner class ShowRenderer(private val showRunner: ShowRunner) : Renderer {
            val shaderBuffers =
                showRunner.allSurfaces.associateWith { showRunner.getShaderBuffer(it, solidShader) }.toMutableMap()

            override fun nextFrame() {
                shaderBuffers.values.forEach { it.color = Color.WHITE }
            }

            override fun surfacesChanged(newSurfaces: List<Surface>, removedSurfaces: List<Surface>) {
                if (!supportsSurfaceChange) {
                    super.surfacesChanged(newSurfaces, removedSurfaces)
                } else {
                    removedSurfaces.forEach { shaderBuffers.remove(it) }
                    newSurfaces.forEach { shaderBuffers[it] = showRunner.getShaderBuffer(it, solidShader) }
                }
            }
        }
    }
}

class StubBeatSource : BeatSource {
    override fun getBeatData(): BeatData = BeatData(0.0, 0, confidence = 0f)
}
