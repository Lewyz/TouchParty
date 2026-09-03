import java.util.Properties

plugins {
    alias(libs.plugins.android.application)
}

val localProperties = Properties().apply {
    val localPropertiesFile = rootProject.file("local.properties")
    if (localPropertiesFile.exists()) {
        load(localPropertiesFile.inputStream())
    }
}

val gameServerHttpUrl = localProperties.getProperty("GAME_SERVER_HTTP_URL") ?: "https://game.tutaxi502.com"
val gameServerWsUrl = localProperties.getProperty("GAME_SERVER_WS_URL") ?: "wss://game.tutaxi502.com"

android {
    namespace = "com.lewyzstudio.touchparty"
    compileSdk = 35

    defaultConfig {
        applicationId = "com.lewyzstudio.touchparty"
        minSdk = 30
        targetSdk = 35
        versionCode = 1
        versionName = "1.0"

        testInstrumentationRunner = "androidx.test.runner.AndroidJUnitRunner"

        buildConfigField("String", "GAME_SERVER_HTTP_URL", "\"$gameServerHttpUrl\"")
        buildConfigField("String", "GAME_SERVER_WS_URL", "\"$gameServerWsUrl\"")

        externalNativeBuild {
            cmake {
                arguments(
                    "-DGAME_SERVER_HTTP_URL=$gameServerHttpUrl",
                    "-DGAME_SERVER_WS_URL=$gameServerWsUrl"
                )
            }
        }
    }

    buildTypes {
        release {
            optimization {
                enable = false
            }
        }
    }
    compileOptions {
        sourceCompatibility = JavaVersion.VERSION_11
        targetCompatibility = JavaVersion.VERSION_11
    }
    buildFeatures {
        prefab = true
        buildConfig = true
    }
    externalNativeBuild {
        cmake {
            path = file("src/main/cpp/CMakeLists.txt")
            version = "3.22.1"
        }
    }
}

dependencies {
    implementation(libs.androidx.appcompat)
    implementation(libs.androidx.core.ktx)
    implementation(libs.androidx.games.activity)
    implementation(libs.material)
    implementation("com.squareup.okhttp3:okhttp:4.12.0")
    testImplementation(libs.junit)
    androidTestImplementation(libs.androidx.espresso.core)
    androidTestImplementation(libs.androidx.junit)
}