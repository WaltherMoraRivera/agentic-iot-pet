package pet

import androidx.compose.ui.window.Window
import androidx.compose.ui.window.application
import pet.ui.PetApp

fun main() = application {
    Window(onCloseRequest = ::exitApplication, title = "Pet IoT") {
        PetApp()
    }
}
