plugins { `java-library`; `maven-publish` }
group = "com.nexvary.npse"
version = "0.1.0-alpha01"
java { sourceCompatibility = JavaVersion.VERSION_17; targetCompatibility = JavaVersion.VERSION_17; withSourcesJar() }
publishing { publications { create<MavenPublication>("core") { from(components["java"]) } } }
val policySmoke by tasks.registering(JavaExec::class) {
    dependsOn(tasks.testClasses)
    classpath = sourceSets["test"].runtimeClasspath
    mainClass.set("com.nexvary.npse.core.EngineTest")
}
tasks.check { dependsOn(policySmoke) }

tasks.withType<Jar>().configureEach { from("LICENSE") { into("META-INF") } }
