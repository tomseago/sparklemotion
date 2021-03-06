package baaahs

import kotlinx.serialization.list
import kotlinx.serialization.map
import kotlinx.serialization.serializer

object Topics {
    val availableShows = PubSub.Topic("availableShows", String.serializer().list)
    val selectedShow = PubSub.Topic("selectedShow", String.serializer())
    val activeGadgets = PubSub.Topic("activeGadgets", GadgetData.serializer().list)
    val movingHeads = PubSub.Topic("movingHeads", MovingHead.serializer().list)

    val movingHeadPresets = PubSub.Topic(
        "movingHeadPresets",
        (String.serializer() to MovingHead.MovingHeadPosition.serializer()).map
    )
}