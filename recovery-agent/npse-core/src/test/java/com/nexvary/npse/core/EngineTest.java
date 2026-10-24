package com.nexvary.npse.core;
import java.io.*;
import java.util.*;
import java.util.concurrent.*;

/** Dependency-free executable security/engine tests, also run by Gradle check. */
public final class EngineTest {
    private static int cases;
    static void verify(boolean condition, String label) { if (!condition) throw new AssertionError(label); cases++; }
    static final class FakeProcess extends Process {
        final InputStream input;
        final boolean never;
        final int exit;
        volatile boolean destroyed;
        final CountDownLatch waiting = new CountDownLatch(1);
        FakeProcess(String data, boolean never, int exit) { this.input = new ByteArrayInputStream(data.getBytes(java.nio.charset.StandardCharsets.UTF_8)); this.never = never; this.exit = exit; }
        public OutputStream getOutputStream() { return new ByteArrayOutputStream(); }
        public InputStream getInputStream() { return input; }
        public InputStream getErrorStream() { return new ByteArrayInputStream(new byte[0]); }
        public int waitFor() { return exit; }
        public boolean waitFor(long time, TimeUnit unit) throws InterruptedException { waiting.countDown(); if (never) { Thread.sleep(Math.min(5, unit.toMillis(time))); return false; } return true; }
        public int exitValue() { return exit; }
        public void destroy() { destroyed = true; }
        public Process destroyForcibly() { destroyed = true; return this; }
    }
    public static void main(String[] args) throws Exception {
        verify(AccessPolicy.evaluate(false,true,2000,false) == AccessPolicy.State.UNAVAILABLE,"binder absent");
        verify(AccessPolicy.evaluate(true,false,2000,false) == AccessPolicy.State.PERMISSION_REQUIRED,"request required");
        verify(AccessPolicy.evaluate(true,false,2000,true) == AccessPolicy.State.DENIED,"denial");
        verify(AccessPolicy.evaluate(true,true,0,false) == AccessPolicy.State.UNSUPPORTED_ROOT,"root refused");
        verify(AccessPolicy.evaluate(true,true,1000,false) == AccessPolicy.State.UNAVAILABLE,"system uid refused");
        verify(AccessPolicy.evaluate(true,true,2000,false) == AccessPolicy.State.SHELL,"shell");
        verify(AccessPolicy.evaluate(true,false,2000,false) != AccessPolicy.State.SHELL,"revocation");
        verify(AccessPolicy.evaluate(false,true,2000,false) != AccessPolicy.State.SHELL,"disconnect");
        verify(AccessPolicy.callerAllowed(10123,10123),"owner caller");
        for (int caller : new int[]{0,2000,1000,10124,-1}) verify(!AccessPolicy.callerAllowed(caller,10123),"foreign caller");
        verify(!AccessPolicy.callerAllowed(2000,2000),"invalid owner");
        for (String id : Arrays.asList(null,"sh","su","id;reboot","$(id)","../../data","shell-identity\n","/system/bin/id")) verify(Probe.find(id) == null,"command injection");
        verify(Probe.find("shell-identity") == Probe.IDENTITY,"known probe");
        boolean immutable = false; try { Probe.IDENTITY.arguments().add(";reboot"); } catch (UnsupportedOperationException e) { immutable = true; }
        verify(immutable,"argv immutable");
        for (Probe p : Probe.values()) verify(p.arguments().get(0).startsWith("/system/bin/") && !p.arguments().contains("-c"), "fixed executable");
        FakeProcess good = new FakeProcess("uid=2000(shell)",false,0);
        BoundedRunner.Result result = new BoundedRunner(argv -> { verify(argv.equals(Probe.IDENTITY.arguments()),"typed argv"); return good; }).run(Probe.IDENTITY,new BoundedRunner.Cancellation());
        verify(result.status == BoundedRunner.Status.OK && result.value.equals("uid=2000(shell)"),"success");
        verify(good.destroyed,"cleanup on success");
        FakeProcess failure = new FakeProcess("secret-error-token",false,1);
        result = new BoundedRunner(argv -> failure).run(Probe.IDENTITY,new BoundedRunner.Cancellation());
        verify(result.status == BoundedRunner.Status.DENIED && result.value.isEmpty(),"stderr not retained");
        FakeProcess oversized = new FakeProcess("x".repeat(4097),false,0);
        result = new BoundedRunner(argv -> oversized).run(Probe.IDENTITY,new BoundedRunner.Cancellation());
        verify(result.status == BoundedRunner.Status.OUTPUT_LIMIT && oversized.destroyed,"output bound");
        FakeProcess stalled = new FakeProcess("",true,0);
        result = new BoundedRunner(argv -> stalled).run(Probe.IDENTITY,new BoundedRunner.Cancellation(),30);
        verify(result.status == BoundedRunner.Status.TIMEOUT && stalled.destroyed,"timeout cleanup");
        BoundedRunner.Cancellation canceled = new BoundedRunner.Cancellation(); canceled.cancel();
        result = new BoundedRunner(argv -> { throw new AssertionError("must not launch"); }).run(Probe.IDENTITY,canceled);
        verify(result.status == BoundedRunner.Status.CANCELED,"prelaunch cancellation");
        BoundedRunner.Cancellation during = new BoundedRunner.Cancellation();
        FakeProcess active = new FakeProcess("",true,0);
        ExecutorService thread = Executors.newSingleThreadExecutor();
        Future<BoundedRunner.Result> future = thread.submit(() -> new BoundedRunner(argv -> active).run(Probe.IDENTITY,during));
        verify(active.waiting.await(1, TimeUnit.SECONDS),"child actually launched before cancel");
        during.cancel(); result = future.get(2,TimeUnit.SECONDS); thread.shutdownNow();
        verify(result.status == BoundedRunner.Status.CANCELED && active.destroyed,"active cancellation destroys child");
        BoundedRunner.Cancellation launchRace = new BoundedRunner.Cancellation();
        FakeProcess late = new FakeProcess("",true,0);
        CountDownLatch launching = new CountDownLatch(1), releaseLaunch = new CountDownLatch(1);
        ExecutorService raceThread = Executors.newSingleThreadExecutor();
        Future<BoundedRunner.Result> race = raceThread.submit(() -> new BoundedRunner(argv -> {
            launching.countDown();
            try { if (!releaseLaunch.await(1,TimeUnit.SECONDS)) throw new IOException("launch barrier timeout"); }
            catch (InterruptedException e) { Thread.currentThread().interrupt(); throw new IOException("interrupted"); }
            return late;
        }).run(Probe.IDENTITY,launchRace));
        verify(launching.await(1,TimeUnit.SECONDS),"launch race entered");
        launchRace.cancel(); releaseLaunch.countDown();
        result = race.get(2,TimeUnit.SECONDS); raceThread.shutdownNow();
        verify(result.status == BoundedRunner.Status.CANCELED && late.destroyed,"startup race destroys late child");
        result = new BoundedRunner(argv -> { throw new SecurityException("secret"); }).run(Probe.IDENTITY,new BoundedRunner.Cancellation());
        verify(result.status == BoundedRunner.Status.DENIED && result.value.isEmpty(),"denial sanitized");
        result = new BoundedRunner(argv -> { throw new IOException("secret"); }).run(Probe.IDENTITY,new BoundedRunner.Cancellation());
        verify(result.status == BoundedRunner.Status.UNAVAILABLE && result.value.isEmpty(),"launch failure sanitized");
        Thread.currentThread().interrupt();
        result = new BoundedRunner(argv -> { throw new AssertionError("must not launch"); }).run(Probe.IDENTITY,new BoundedRunner.Cancellation());
        verify(result.status == BoundedRunner.Status.CANCELED,"thread interruption"); Thread.interrupted();
        System.out.println("NPSE core: " + cases + " assertions passed");
    }
}
