# Execution checkpoint — 2026-10-16

Development branch dev/cpp-qt-foundation; PR1 draft; main unchanged.

Latest externally supplied revision5966e81 passed Windows/Qt run38925671316 but failed Python run38925670781: a required os import was removed while os.fstat remained in use. Fix45d7698 restores it without reverting JPEG/EXIF support. Both [Python38943229397](https://github.com/nexvary/NEXVARY-MR/actions/runs/38943229397) and [Windows38943229435](https://github.com/nexvary/NEXVARY-MR/actions/runs/38943229435) passed for45d7698.

The subsequent0.4.0 development increment integrates the earlier native recovery work onto that actual branch head, preserving the nine-page navigation, colored symbols, accessible controls, source assessment and research Python suites. Source assessment remains a separate tab; a native case/image workflow is added alongside it. Do not replace the current branch with the old local checkpoint d6606f9/7aa55f3.

Local native compilation with warnings as errors succeeded; CTest9/9 passed before the additional EXIF/native image regression and screenshot extension. The EXIF suite then passed9/9 QtTest results, UI13/13, and extended GUI smoke passed with actual synthetic candidates and a second recovery PNG. Python research19/19 and legacy4/4 passed. Linux fixture cases are explicitly unencrypted test profiles, not production vault acceptance.

Android source/wrapper/CI exist. Local Gradle dependency resolution encountered a network proxy refusal; no local APK/build success is claimed. The new exact-revision Windows DPAPI/packaging and Android build/emulator workflows must be checked before claiming validation. The source state and workflow files alone are not proof of success.

Physical OEM/Android/USB/scrcpy/Shizuku, clean consumer Windows/DPI/accessibility, licensed phone boards, actual reset experiments, publisher signing and privacy/license acceptance remain external gates. Native image analysis is bounded contiguous PNG/JPEG, not a filesystem parser, EWF decoder, decryption engine or deleted-file certainty. See RECOVERY_GUIDE.md and the research directory for precise limits.
