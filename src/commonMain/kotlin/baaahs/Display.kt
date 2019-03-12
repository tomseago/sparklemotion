package baaahs

expect fun getDisplay(): Display

interface Display {
    fun forNetwork(): NetworkDisplay
    fun forPinky(): PinkyDisplay
    fun forBrain(): BrainDisplay
    fun forMapper(): MapperDisplay
}

interface NetworkDisplay {
    fun receivedPacket()
    fun droppedPacket()
}

interface PinkyDisplay {
    var brainCount: Int
    var beat: Int
}

interface BrainDisplay {
    fun haveLink(link: Network.Link)
}

interface MapperDisplay