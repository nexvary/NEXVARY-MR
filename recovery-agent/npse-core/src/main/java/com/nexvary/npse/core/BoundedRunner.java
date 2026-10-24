package com.nexvary.npse.core;
import java.io.ByteArrayOutputStream;
import java.io.IOException;
import java.nio.charset.StandardCharsets;
import java.util.List;
import java.util.concurrent.*;
import java.util.concurrent.atomic.AtomicBoolean;

/** Shared subprocess engine, also used by the existing recovery metadata service. */
public final class BoundedRunner {
    public static final int MAX_BYTES = 4096;
    public enum Status { OK, DENIED, UNAVAILABLE, TIMEOUT, CANCELED, OUTPUT_LIMIT }
    public static final class Result {
        public final Status status;
        public final String value;
        public Result(Status status, String value) { this.status = status; this.value = value; }
    }
    public static final class Cancellation {
        private final AtomicBoolean canceled = new AtomicBoolean();
        private final java.util.concurrent.atomic.AtomicReference<Process> child = new java.util.concurrent.atomic.AtomicReference<>();
        public void cancel() { canceled.set(true); Process p = child.get(); if (p != null) p.destroyForcibly(); }
        private void attach(Process p) { child.set(p); if (canceled.get()) p.destroyForcibly(); }
        private void detach(Process p) { child.compareAndSet(p, null); }
        public boolean isCanceled() { return canceled.get(); }
    }
    @FunctionalInterface interface Launcher { Process start(List<String> argv) throws IOException; }
    private final Launcher launcher;
    public BoundedRunner() { this(argv -> new ProcessBuilder(argv).redirectErrorStream(true).start()); }
    BoundedRunner(Launcher launcher) { this.launcher = launcher; }
    public Result run(Probe probe, Cancellation cancellation) { return run(probe, cancellation, 1500); }
    Result run(Probe probe, Cancellation cancellation, long timeoutMs) {
        if (probe == null || cancellation == null) throw new IllegalArgumentException("Typed operation required");
        if (cancellation.isCanceled() || Thread.currentThread().isInterrupted()) return result(Status.CANCELED);
        Process child = null;
        ExecutorService reader = Executors.newSingleThreadExecutor(r -> { Thread t = new Thread(r, "npse-output"); t.setDaemon(true); return t; });
        try {
            child = launcher.start(probe.arguments());
            cancellation.attach(child);
            Process process = child;
            Future<String> output = reader.submit(() -> {
                ByteArrayOutputStream bytes = new ByteArrayOutputStream();
                try (var input = process.getInputStream()) {
                    byte[] buffer = new byte[512];
                    for (int count; (count = input.read(buffer)) != -1;) {
                        if (bytes.size() + count > MAX_BYTES) { process.destroyForcibly(); throw new OutputLimit(); }
                        bytes.write(buffer, 0, count);
                    }
                }
                return new String(bytes.toByteArray(), StandardCharsets.UTF_8).trim();
            });
            long deadline = System.nanoTime() + TimeUnit.MILLISECONDS.toNanos(timeoutMs);
            while (!process.waitFor(25, TimeUnit.MILLISECONDS)) {
                if (cancellation.isCanceled()) return result(Status.CANCELED);
                if (System.nanoTime() >= deadline) return result(Status.TIMEOUT);
            }
            if (cancellation.isCanceled()) return result(Status.CANCELED);
            String text;
            try { text = output.get(Math.max(1, TimeUnit.NANOSECONDS.toMillis(deadline - System.nanoTime())), TimeUnit.MILLISECONDS); }
            catch (ExecutionException e) { return result(e.getCause() instanceof OutputLimit ? Status.OUTPUT_LIMIT : Status.UNAVAILABLE); }
            if (process.exitValue() != 0) return result(Status.DENIED); // Never retain raw stderr.
            return new Result(Status.OK, text);
        } catch (InterruptedException e) { Thread.currentThread().interrupt(); return result(Status.CANCELED); }
        catch (TimeoutException e) { return result(Status.TIMEOUT); }
        catch (SecurityException e) { return result(Status.DENIED); }
        catch (IOException e) { return result(Status.UNAVAILABLE); }
        finally {
            if (child != null) {
                cancellation.detach(child);
                child.destroyForcibly();
                try { child.getInputStream().close(); } catch (IOException ignored) {}
                try { child.getOutputStream().close(); } catch (IOException ignored) {}
                try { child.getErrorStream().close(); } catch (IOException ignored) {}
            }
            reader.shutdownNow();
        }
    }
    private static Result result(Status s) { return new Result(s, ""); }
    private static final class OutputLimit extends IOException { private static final long serialVersionUID = 1L; }
}
