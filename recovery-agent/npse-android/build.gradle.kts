plugins { id("com.android.library"); id("org.jetbrains.kotlin.android"); `maven-publish` }
group = "com.nexvary.npse"
version = "0.1.0-alpha01"
android {
    namespace = "com.nexvary.npse"
    compileSdk = 35
    defaultConfig { minSdk = 26; testInstrumentationRunner = "androidx.test.runner.AndroidJUnitRunner"; consumerProguardFiles("consumer-rules.pro") }
    buildFeatures { aidl = true }
    compileOptions { sourceCompatibility = JavaVersion.VERSION_17; targetCompatibility = JavaVersion.VERSION_17 }
    kotlinOptions { jvmTarget = "17" }
    publishing { singleVariant("release") { withSourcesJar() } }
    lint { abortOnError = true }
}
dependencies {
    api(project(":npse-core"))
    implementation("dev.rikka.shizuku:api:13.1.5")
    implementation("dev.rikka.shizuku:provider:13.1.5")
    testImplementation("junit:junit:4.13.2")
    testImplementation("org.json:json:20240303")
}
afterEvaluate { publishing { publications { create<MavenPublication>("release") { from(components["release"]) } } } }
