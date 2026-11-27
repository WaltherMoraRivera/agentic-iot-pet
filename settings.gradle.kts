pluginManagement {
    repositories {
        google()
        mavenCentral()
        gradlePluginPortal()
    }
}

dependencyResolutionManagement {
    repositoriesMode.set(RepositoriesMode.FAIL_ON_PROJECT_REPOS)
    repositories {
        google()
        mavenCentral()
    }
}

rootProject.name = "pet-iot"

include(":shared", ":androidApp", ":desktopApp", ":simulator")
project(":shared").projectDir = file("apps/shared")
project(":androidApp").projectDir = file("apps/androidApp")
project(":desktopApp").projectDir = file("apps/desktopApp")
