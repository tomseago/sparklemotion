package baaahs

import baaahs.shows.CompositeShow
import baaahs.shows.RandomShow
import baaahs.shows.SomeDumbShow
import kotlinx.coroutines.CoroutineScope
import kotlinx.coroutines.GlobalScope
import kotlinx.coroutines.delay
import kotlinx.coroutines.launch
import kotlin.coroutines.CoroutineContext
import kotlin.coroutines.EmptyCoroutineContext

class SheepSimulator {
    var display = getDisplay()
    var network = FakeNetwork(display = display.forNetwork())

    var dmxUniverse = FakeDmxUniverse()
    var sheepModel = SheepModel()

    val showMetas = listOf(
        SomeDumbShow.Meta(),
        RandomShow.Meta(),
        CompositeShow.Meta()
    )

    val visualizer = Visualizer(sheepModel, dmxUniverse).also {
        it.onStartMapper = {
            it.setMapperRunning(true)
            Mapper(network, sheepModel, it.mediaDevices).apply {
                this.addCloseListener({ it.setMapperRunning(false) })
                start()
            }
        }
    }

    val pinky = Pinky(sheepModel, showMetas, network, dmxUniverse, display.forPinky())

    fun start() {
        sheepModel.load()

        PinkyScope.launch { pinky.run() }

        visualizer.start()

        sheepModel.panels.forEach { panel ->
            val jsPanel = visualizer.showPanel(panel)
            val brain = Brain(network, display.forBrain(), JsPixels(jsPanel), panel)
            BrainScope.launch { randomDelay(1000); brain.run() }
        }

        sheepModel.eyes.forEach { eye ->
            visualizer.addEye(eye)
            Config.DMX_DEVICES[eye.name]
        }

        GlobalScope.launch {
            Ui(network, pinky.address, display.forUi())
        }

        doRunBlocking {
            delay(200000L)
        }
    }

    object PinkyScope : CoroutineScope {
        override val coroutineContext: CoroutineContext
            get() = EmptyCoroutineContext
    }

    object BrainScope : CoroutineScope {
        override val coroutineContext: CoroutineContext
            get() = EmptyCoroutineContext
    }
}
