#-------------------------------------------------------------------------------
AAPT ?= aapt
ZIPALIGN ?= zipalign
APKSIGNER ?= apksigner

ANDROID_APK_BUILD_PATH = $(GAME_BUILD_PATH)/apk
ANDROID_ARCH ?= arm64-v8a
ANDROID_JAR ?=

ANDROID_SIGN_KEY ?= android/debug.pk8
ANDROID_SIGN_CERT ?= android/debug.x509.pem


#-------------------------------------------------------------------------------
GAME_LIB = $(GAME_BUILD_PATH)/apk/lib/$(ANDROID_ARCH)/lib$(GAME_NAME).so


#-------------------------------------------------------------------------------
$(GAME_NAME).apk: $(GAME_BUILD_PATH)/$(GAME_NAME).unsigned.apk
	$(APKSIGNER) sign --key $(ANDROID_SIGN_KEY) --cert $(ANDROID_SIGN_CERT) --out $@ $<

$(GAME_BUILD_PATH)/$(GAME_NAME).unsigned.apk: $(GAME_BUILD_PATH)/$(GAME_NAME).unsigned.unaligned.apk
	$(ZIPALIGN) -f -p 4 $< $@

$(GAME_BUILD_PATH)/$(GAME_NAME).unsigned.unaligned.apk: $(GAME_LIB)
	$(AAPT) package -f -M android/AndroidManifest.xml -S android/res -A assets \
		-I $(ANDROID_JAR) -F $@ $(ANDROID_APK_BUILD_PATH)

$(GAME_LIB): $(OBJECTS)
	$(call mkdir,$(@D))
	$(CC) $^ $(LDFLAGS) $(LDLIBS) -o $@

