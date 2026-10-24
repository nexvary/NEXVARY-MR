# NEXVARY-MR — مصفوفة المتطلبات والفجوات

تاريخ الفحص: 2026-10-23. خط الأساس المقروء: `7c928b0d5cc3842964dcbe9f207971c09af588b5`، فرع `integration/gate-1-stages-08-60`، PR [#8](https://github.com/nexvary/NEXVARY-MR/pull/8) مسودة ويستهدف `feature/device-service-engine`. لم يُدمج شيء في main. هذه مصفوفة قدرات محددة، وليست نسبة اكتمال تجاري أو ترقية إلى المرحلة 60.

المصادر: طلب المستخدم الحالي، المتطلبات الظاهرة والمستعادة من سجل المحادثات المتاح، `REQUIREMENTS.md`، `release/INTEGRATION_GATE_08_60.md`، وثائق الخدمات والبحث، الشفرة والاختبارات الحالية، API العام للفروع وPRs وIssues وActions. لم يُفترض الاطلاع على محادثات غير متاحة. لا توجد Issues مستقلة مفتوحة عند الفحص؛ نتائج endpoint الخاص بالـ Issues كانت PRs #1–#8.

`COMPLETE` = القدرة المحددة منفذة ومتاحة ومختبرة ضمن النطاق المذكور فقط؛ `PARTIAL` = المتطلب الأوسع غير مغلق؛ `MISSING` = لا تنفيذ؛ `BLOCKED` = عائق مثبت؛ `HARDWARE-PENDING` = اختبار جهاز/بيئة فعلية مطلوب. الأولوية P0 للاستقرار والإصدار، P1 للعمل اليومي، P2 للتوسع.

## المصفوفة التنفيذية

|المتطلب|الحالة الحالية|الدليل|المتبقي|الأولوية|اختبار القبول|
|---|---|---|---|---|---|
|01. بناء C++/Qt على Windows وLinux لخط الأساس|COMPLETE|Actions [40000060143](https://github.com/nexvary/NEXVARY-MR/actions/runs/40000060143): 7 وظائف ناجحة على SHA أعلاه|إعادة CI لكل تعديل لاحق؛ النجاح لا ينتقل إلى commit جديد|P0|كل وظائف C++/Qt للـ SHA المراد إصداره ناجحة|
|02. تحديد فشل التشغيل 39896846776|COMPLETE|[تعليق check-run](https://api.github.com/repos/nexvary/NEXVARY-MR/check-runs/120229161140/annotations): منع البدء بسبب مدفوعات الحساب/حد الإنفاق؛ لا خطوات ولا runner|لا تعديل شفرة يعالج سبب الفشل التاريخي؛ التشغيل الأحدث ناجح|P0|دليل annotation محفوظ في وثيقة تدقيق الجلسة|
|03. اختبارات Python الحالية|COMPLETE|`.github/workflows/test.yml`؛ [40000059901](https://github.com/nexvary/NEXVARY-MR/actions/runs/40000059901) ناجح لخط الأساس|محليًا 27 اختبار أساس +6 portal ناجحة؛ CI للتعديل الجديد مطلوب|P0|unittest ناجح مع بيان skips|
|04. منع كتابة الهاتف افتراضيًا والموافقات|COMPLETE|`ServiceOperationPolicy.hpp`، `ServiceCommandPolicy.hpp`، `policy_tests.cpp` و`service_tests.cpp`؛ production/Laboratory CI ناجح|مراجعة مستقلة قبل الترويج|P0|منع العملية قبل QProcess في production، تأكيدات وهوية حديثة في Laboratory|
|05. ADB/Fastboot: الاكتشاف والاختيار والمهل والإلغاء|HARDWARE-PENDING|`ToolBridge.cpp`، `DeviceProtocol.hpp`، `tool_tests.cpp`|مصفوفة هواتف وUSB/OEM وانقطاع وإعادة اتصال فعلية|P1|الجهاز المحدد وحده، رفض الالتباس/unauthorized، انتهاء العملية بأمان على هاتف مصرح|
|06. تشخيص البطارية والتخزين والأقسام|HARDWARE-PENDING|`DeviceHealth.hpp`، `DiagnosisPage.qml`، اختبارات tool/service|مقارنة المشاهدات بمرجع فعلي لكل موديل وبناء|P1|قيم مرجعية موثقة؛ unknown صريح عند غياب الدليل|
|07. USB وRecovery وربط تعريف Windows|HARDWARE-PENDING|`UsbDriver.hpp`، `packaging/inspect-usb.ps1`، `USB_CORRELATION.md`|اختبار binding وRecovery وFastbootd على أجهزة فعلية|P1|توثيق PnP والهوية وحالة النقل دون ادعاء أن التطابق النصي تعريف معتمد|
|08. scrcpy: دورة العملية وواجهة البدء والإيقاف|HARDWARE-PENDING|`MirrorSession.cpp`، `mirror_tests.cpp`، `DevicesPage.qml`؛ موجود قبل الجلسة|عرض إطارات وإغلاق وتنظيف مكون الهاتف وانقطاع USB على Windows/هاتف فعلي|P1|5.0.1، موافقة صريحة، هدف مثبت، إيقاف العملية؛ production يرفض|
|09. تشخيص OEM وكتالوج الموديلات|PARTIAL|`ServiceCatalog.cpp`، `DeviceServices.cpp`، بيانات 14 مجموعة/44 مرجع موديل|وحدات تشخيص أصلية أكثر واعتماد موديلات؛ الكتالوج لا يمنح دعمًا|P1|كل عملية لها مصدر/خطة/اختبار؛ لا أوامر محقونة من البيانات|
|10. إدارة التطبيقات والنسخ/الاستعادة المدعومة|PARTIAL|نسخ APK خاصة متحقق منها وخدمات Laboratory؛ أدلة `IMPLEMENTATION.md`|مسار نسخ احتياطي/استعادة متكامل معلوم النطاق، تحقق ما بعد العملية وبيانات اختبار|P1|استعادة بيانات تجريبية مدعومة ومقارنتها؛ عدم الوعد بنسخة كاملة لكل التطبيقات|
|11. انتقالات/إصلاحات الهاتف ذات الكتابة|HARDWARE-PENDING|خدمات Laboratory مع target/consent/policy؛ لا production writes|قبول كل عملية وموديل والتحقق من النتيجة الفعلية|P1|ACK لا يكفي؛ حالة بعدية موثقة دون تجاوز حماية|
|12. تحليل حاويات Firmware والبيانات الوصفية|COMPLETE|`PackagePreflight.hpp`، `FirmwareBridge.cpp`، `firmware_tests.cpp`، تبويب الحزم|توسعة أنواع الحزم في بند منفصل؛ هذه بنية ZIP فقط|P1|رفض المسارات/الأسماء/الأطوال التالفة؛ تقرير structure-only متاح|
|13. SHA-256 ومطابقة manifest|COMPLETE|`FirmwareBridge.cpp` واختبارات firmware والإلغاء|توثيق أن manifest غير الموثق ليس مصادقة OEM|P1|تطابق بصمة معلوم وتغير المصدر/الإلغاء يرفضان النتيجة|
|14. AVB المحلي: بيانات vbmeta والأقسام المعلنة|COMPLETE|`AvbMetadata.cpp`، `avb_tests.cpp`، مرجع avbtool واختبارات mutation وتبويب AVB|توقيع/OEM trust/full package verification غير منفذ|P1|قبول المرجع ورفض التالف؛ لا compatible أو signature verified مستنتج|
|15. قواعد توافق Firmware وتقارير مخاطر الإصلاح|PARTIAL|`DeviceCompatibility.hpp`، قواعد هوية/بناء وتشفير وصفحات Firmware|دمج صور الأقسام وOEM signatures وسلاسل الثقة وملفات مخاطر أكثر|P1|أزواج موديل/بناء معروفة؛ تعارض الهوية/التوقيع يمنع اعتماد التوافق|
|16. استيراد أدلة محلية مصرح بها وحفظها|COMPLETE|`RecoveryBridge.cpp`، `CaseVault.cpp`، `recovery_tests.cpp` و`forensic_tests.cpp`|مراجعة سياسة الخصوصية الأوسع منفصلة|P1|عدم تعديل المصدر؛ SHA-256، إذن، إعادة فتح وتصدير مشروط|
|17. سلسلة الحيازة المحلية والتقارير|PARTIAL|`ForensicRecord.cpp`؛ سلسلة hashes، نقل حيازة معلن، PDF/HTML/JSON|توقيع/مرساة مستقلة واعتماد إجرائي؛ السجل المحلي ليس إثبات تسليم فعلي|P1|كشف التعديل والترتيب؛ عدم نسب دليل قديم لمشغّل جديد|
|18. EXIF من JPEG في مختبر الأدلة|PARTIAL|تنفيذ الجلسة: محرك وواجهة؛ EXIF 18 وQML 34، CTest 29/29 لكل profile؛ أدلة `EXIF_INCREMENT_2026-10-23.md`|Windows CI للـ commit الجديد والاعتماد المطلوب غير متاحين عبر الاتصال الحالي|P1|LE/BE، مرجع Pillow، malformed/mutations، إذن، حفظ/فتح/تصدير، نقر AR/EN|
|19. تحليل النسخ الاحتياطية المصرح بها|MISSING|استيراد ملفات/Agent متاح؛ لا parser خاص بنسخ Android/iOS|ابدأ بصيغة محددة ونسخة تجريبية ومخطط artifact؛ لا فك حماية|P1|أثر معروف داخل نسخة ثابتة مع بصمة ومقارنة نتيجة مستقلة|
|20. دمج ALEAPP|MISSING|`ToolBridge.cpp` اكتشاف مرشح فقط؛ `OPEN_SOURCE_SELECTION_2026-10-23.md`|محول خارجي bounded، مراجعة نسخة/ترخيص/dependencies، fixtures وإلغاء|P2|نتائج artifacts معروفة؛ لا تشغيل plugins داخل workstation أو استخراج غير مصرح|
|21. تحليل أنظمة الملفات واستعادة الملفات|PARTIAL|`RecoveryImage.cpp` محدود PNG/JPEG؛ `TskImage.cpp` mmls على نسخة متحقق منها|mmls قائمة أقسام وليس parser F2FS/ext4؛ لا استعادة داخلية بعد فقد المفاتيح|P2|صور تجريبية معلومة ومقارنة أدوات مستقلة دون تغيير المصدر|
|22. PCAP المحلي: إحصاءات محدودة|COMPLETE|`PcapSummary.cpp`، `pcap_tests.cpp`، تبويب الأدوات واختبارات QML|PCAPNG والـ flows في بند توسعة مستقل|P1|endianness/time/length bounds، إلغاء، لا payload أو عناوين في الملخص|
|23. PCAPNG وflow reconstruction|MISSING|حدود `docs/forensics/PCAP_SUMMARY.md` صريحة|قارئ bounded أو محول TShark خارجي بعد مراجعة|P2|captures معلومة، negative cases، مقارنة counts مستقلة|
|24. إدارة العملاء والأجهزة وأوامر الصيانة والبحث|COMPLETE|`WorkshopBridge.cpp` و`WorkshopPage.qml` واختبارات workshop/center|قبول ميداني أوسع منفصل|P1|CRUD ومسارات البحث وحالات الطلب ومعاملات SQLite|
|25. مخزون وأسعار وتخصيص قطع ذري|COMPLETE|اختبارات workshop: integer money، rollback، stock allocation|قبول محاسب/ورشة|P1|رفض المخزون غير الكافي وحفظ الأسعار التاريخية وrollback|
|26. باركود ثابت لكل أمر وبطاقة جهاز|COMPLETE|schema 4، `Code39Label.hpp`، `barcode_tests.cpp` وQML؛ موجود قبل الجلسة|قارئ وطابعة فعليان منفصلان|P1|lookup دقيق، ترحيل/backup/restore، PDF وdecoder مستقل|
|27. طباعة الفواتير والبطاقات على جهاز فعلي|HARDWARE-PENDING|PDF متاح والطباعة بقارئ PDF خارجي؛ ليست native spooler|قبول طابعة/ماسح باركود وAR/EN على Windows|P1|مسح الورقة يفتح الأمر الصحيح؛ العربية والأرقام واضحة|
|28. نسخ Workshop الاحتياطية والاستعادة|COMPLETE|`WorkshopBridge.cpp` واختبارات ترحيل/استعادة/snapshot|خطة دورية واحتفاظ واختبار عميل|P1|نسخة سليمة تستعيد العلاقات؛ نسخة فاسدة لا تستبدل القاعدة|
|29. حماية بيانات العملاء وسياسة الاحتفاظ|PARTIAL|DPAPI للقضايا/المفاتيح، redaction، backup؛ CRM SQLite محلي|وصول/احتفاظ/حذف/تشفير CRM وقبول خصوصية شامل|P0|مراجعة ملفات وسجلات ونسخ احتياطية وexport؛ لا بيانات عميل في artifacts|
|30. الهوية البصرية والأيقونات الملونة وAR/EN|PARTIAL|`docs/ui/REBUILD_ACCEPTANCE.md`؛ AppShell و12 قسمًا و24 SVG ملونة؛ Qt/QML 37 نتيجة و31 CTest محليًا|مراجعة بشرية Windows scaling وaccessibility؛ SVG ملونة فعلية وليست صور واجهات تخيلية|P1|1366×768 و1920×1080 و2560×1440، AR/EN و100/125/150%، glyphs، قبول Windows بشري|
|31. التنقل والرجوع والإغلاق والأزرار المختبرة|COMPLETE|`tst_navigation.qml`، GUI smoke، confirmation/keyboard/back|قبول يدوي أوسع؛ لا ادعاء فحص كل مسار ممكن|P1|النقر الحقيقي للمسارات المحددة، رفض/قبول الخروج بلا QML warnings|
|32. Help وAbout ودليل التشغيل|PARTIAL|`HelpPage.qml` و`ReportsPage.qml`، USER_GUIDE وTHIRD_PARTY؛ مساران جديدان مختبران|قبول تحرير/ترجمة بشري وتوثيق المشغّل|P1|الأزرار تعرض معلومات صحيحة ومصادر/تراخيص دون روابط معطلة|
|33. حزمة Windows Portable وInstaller لخط الأساس|COMPLETE|Artifacts فعلية من 40000060143: portable 12522476239 وinstaller 12522830570؛ isolated/install/uninstall jobs ناجحة|ليستا حزم تعديل EXIF؛ بناء commit الجديد مطلوب|P0|isolated path/SDK-free runtime + تثبيت/تشغيل/إزالة ناجحة|
|34. Windows 10/11 نظيف واختبار scaling/accessibility|HARDWARE-PENDING|GitHub hosted Windows فقط؛ Desktop Commander لا أجهزة متصلة|VM نظيف/حاسوب اختبار مستقل وتوثيق القبول|P0|10/11، 100/125/150/200%، تثبيت/portable/خروج/إزالة|
|35. توقيع Authenticode وحماية مفتاح الناشر|BLOCKED|لا وسيلة توقيع/شهادة ناشر متاحة في الجلسة؛ الحزم unsigned عمدًا|شهادة/خدمة وتحقق publisher مستقل|P0|Get-AuthenticodeSignature صالح ومطابق للناشر والـ SHA|
|36. تحقق تحديثات موقعة وسجل محلي|PARTIAL|`UpdateVerifier.cpp`، `UpdateJournal.cpp`، `UpdateKeyStore.cpp` واختبارات؛ مكتبة داخلية مقصودة|مرساة anti-replay محمية، updater UI وتنفيذ installer والتحقق/الرجوع|P0|رفض توقيع/قناة/حزمة خاطئة؛ crash/concurrency/replay؛ ثم end-to-end upgrade|
|37. تدقيق اعتماديات وSBOM وقابلية تكرار البناء|PARTIAL|SPDX file inventory، pinned deps، `THIRD_PARTY.md`|transitive SBOM/license review وإثبات reproducibility؛ NOASSERTION غير معتمد|P0|dependencies والتراخيص كاملة ومراجعة وbuild comparison موثق|
|38. إصدار GitHub Release نهائي|BLOCKED|release_gate يرفض غياب أدلة القبول؛ صلاحية فرع التطوير متاحة الآن|إغلاق بوابات Windows/hardware/privacy/signing/reviewer وإتاحة نشر مصرح|P0|exact-head CI وكل أدلة RC ثم نشر حزم مطابقة؛ لا نشر الآن|
|39. منافسة Chimera/Hydra/UnlockTool/Octoplus|PARTIAL|`device-services/COMPETITIVE_ENGINEERING.md` مقارنة وحدود وكتالوج|benchmarks مماثلة وموديلات مقبولة ووظائف إصلاح أصلية؛ لا تكافؤ تجاري مثبت|P2|مجموعة بيانات/مهمات مشتركة، إصدار/نظام/مقياس نجاح معلوم|
|40. دراسة Cellebrite/Graykey/MSAB/Oxygen/Magnet|PARTIAL|`research/COMPETITIVE_EVIDENCE_2026-10-23.md` مصادر أصلية وتمييز vendor/independent|اختبارات artifacts قابلة للمقارنة؛ لا نقل تقنيات ملكية أو bypass|P2|Research→Feasibility→Prototype→Test→Integration بدليل لكل قدرة|
|41. AVB/TEE/TrustZone/KeyMint/FBE/Qualcomm/MTK/Knox/SEP|PARTIAL|`research/advanced-mobile/knowledge.json`، توافق وتقييم فقد المفاتيح وAVB|نماذج تشخيص/حزم أصلية إضافية؛ ليس أدوات EDL/BROM لتجاوز الحماية|P2|مصادر أولية، prototype تشخيصي read-only، synthetic ثم exact-model acceptance|
|42. دمج مفتوح المصدر بتراخيص واضحة|PARTIAL|scrcpy/ClamAV/mmls external adapters، نسب Code39 وregister|تدقيق اعتماديات وإصدارات Windows وتكاملات منتقاة إضافية|P1|نسخة مقيدة ومصدر/ترخيص وأثر تشغيل واختبارات؛ discovery ليس integration|
|43. بوابة المرحلة 60 وقرار RC|BLOCKED|`INTEGRATION_GATE_08_60.md`، `scripts/release_gate.py`|الأجهزة النظيفة/الفعلية والتوقيع والمراجع وupgrade/privacy؛ لا رفع رقم المرحلة|P0|قبول الأدلة للـ SHA النهائي دون skips غير معلنة|
|44. رفع تغييرات الجلسة والتحقق من CI|PARTIAL|GitHub connector عاد للعمل لحساب NEXVARY؛ الفحص المحلي 31/31؛ Desktop Commander بلا أجهزة|نشر فرع المراجعة والتحقق من CI للشفرة الجديدة|P0|push/PR ناجحان ثم exact-head CI؛ لا افتراض نشر محلي|

## توجيه الأولويات — 2026-10-24

تطوير ميزات جنائية جديدة موقوف مؤقتًا؛ الموجود يبقى مستقرًا مع إصلاحات حرجة فقط.
الأولوية الجديدة للصيانة وUSB ومعلومات الجهاز وFirmware وOEM والتدريب.
شريحة USB Doctor الحالية موثقة في `device-services/USB_DOCTOR.md`، مع فصل
اختبارات التفسير الاصطناعية عن قبول Windows والهاتف الفعلي.

|المتطلب|الحالة الحالية|الدليل|المتبقي|الأولوية|اختبار القبول|
|---|---|---|---|---|---|
|45. USB Doctor: تفسير اتصال بمصدر وعمر قراءة ونطاق هدف|PARTIAL|ConnectionDiagnosis، ToolBridge، UsbDoctorPanel، اختبارات connection/tool/QML|CI للـcommit الجديد، الهاتف والكابل الفعليان، سجل الانقطاعات والواصفات في شرائح مستقلة|P1|رفض القديم/الملتبس، عدم نسبة أخطاء جهاز آخر للهدف، AR/EN، Windows PnP دون هاتف|
|46. Professional Training Simulator|MISSING|الخطة النشطة ELITE_LAB_SCOPE؛ لا تنفيذ يدعى جاهزًا|محرك سيناريوهات معزول وتقييم وقرارات وتقارير|P1|لا QProcess/USB للشاشة التدريبية؛ تقييم معلوم لعينات اصطناعية|

## ترتيب التنفيذ

1. لا إصلاح تخميني لـ CI القديم؛ أدلة الخط الأساس الأخضر محفوظة. كل تعديل جديد يلزمه بناء واختبارات خاصة به.
2. إغلاق شريحة EXIF المحدودة باختبارات محرك/تكامل/واجهة/sanitizers، ثم حفظ commit ونقطة استئناف.
3. النسخ الاحتياطية: صيغة واحدة وأثر معروف قبل التوسع؛ ثم ALEAPP بمحول مستقل، ثم PCAPNG.
4. سياسة خصوصية CRM، اكتمال SBOM، updater/upgrade acceptance؛ هذه أعمال مستقلة عن الهاتف.
5. قبول Windows نظيف والأجهزة/OEM/scrcpy والطباعة، ثم التوقيع والمراجع وRC. لا تُمنع أعمال البرمجيات المستقلة بسبب هذه البوابات.

## الفروع والـ PRs التي فُحصت

|الفرع|HEAD عند الفحص|PR|
|---|---|---|
|main|7446b17d0d83d4e7d9da3c8d4bbdd7dae6d4de17|—|
|dev/cpp-qt-foundation|0bfe9b113cfac6c9f50f0e278d6faa8ff6733ffc|#1|
|feature/oem-service-catalog|ef302c9555dd4fc0b5189182be5bd109d932bbdf|#2|
|feature/forensic-lab-foundation|0f979797f03cabf0dc0a504f23dc5c5f9c2cd116|#3|
|feature/device-service-engine|ac186f4ce569958401e1c4d63479da0553d26c1e|#4|
|feature/structured-device-diagnostics|b0c10df29ba46123ecb673994c366ef7f4fdb1d6|#5|
|feature/windows-usb-correlation|ae05d4fc6e83303239c9e6c751ec98de9b6d5abd|#6|
|feature/metallic-card-contrast|f2da5ab84c844c0921bc2d320e433a0dfe94fe54|#7|
|integration/gate-1-stages-08-60|7c928b0d5cc3842964dcbe9f207971c09af588b5|#8|
|feature/update-journal-checkpoints|7c928b0d5cc3842964dcbe9f207971c09af588b5|—|

الفروع السابقة ليست إصدارات مستقلة متكافئة؛ #5/#6/#7 مدمجة في فرع التكامل قبل هذه الجلسة، وmain ما يزال أساس Python القديم. أحدث native source والـ Actions هما مرجع هذه المصفوفة، لا التقارير القديمة.

## حصيلة الحالة عند نقطة الحفظ

من أصل 44 بندًا: **15 COMPLETE، 15 PARTIAL، 3 MISSING، 4 BLOCKED، 7 HARDWARE-PENDING**. EXIF متحقق محليًا لكنه يبقى PARTIAL لغياب Windows CI للتعديل الجديد. لا تُجمع هذه البنود كنسبة جاهزية أو كعدد مراحل معتمدة. تفاصيل التنفيذ والاختبارات والـ skips في `release/EXIF_INCREMENT_2026-10-23.md`؛ الاستئناف في `../DEVELOPMENT_STATE.md`.
