#Finder for the Google Mobile Ads (AdMob) iOS SDK.
#
#Usage: set GOOGLE_MOBILE_ADS_SDK_ROOT to the directory containing the .xcframework files.
#Download the SDK from: https://developers.google.com/admob/ios/download
#Extract the zip and point this variable at the resulting folder.
#
#If installing via CocoaPods, the dependencies are managed automatically.
#If installing manually, some dependencies may not be present (they're optional).
#
#Sets:
#  GOOGLE_MOBILE_ADS_FOUND        - TRUE if the main Google Mobile Ads framework was located
#  GOOGLE_MOBILE_ADS_FRAMEWORKS   - List of full paths to the .xcframework bundles found

message("finding GoogleMobileAds")

set(GOOGLE_MOBILE_ADS_SDK_ROOT "" CACHE PATH "Path to the extracted Google Mobile Ads SDK directory containing the .xcframework files")

set(GOOGLE_MOBILE_ADS_FRAMEWORKS "" CACHE STRING "" FORCE)

#Required frameworks — must be present
set(_GAD_REQUIRED_FRAMEWORKS
    GoogleMobileAds
)

#Optional frameworks — only added if present
set(_GAD_OPTIONAL_FRAMEWORKS
    UserMessagingPlatform
    FBLPromises
    GoogleAppMeasurement
    GoogleAppMeasurementIdentitySupport
    GoogleUtilities
    nanopb
)

#Check for required frameworks
set(_gad_all_required_found TRUE)
foreach(_fw ${_GAD_REQUIRED_FRAMEWORKS})
    if(NOT EXISTS "${GOOGLE_MOBILE_ADS_SDK_ROOT}/${_fw}.xcframework")
        set(_gad_all_required_found FALSE)
        message(WARNING "GoogleMobileAds: required framework ${_fw}.xcframework not found in '${GOOGLE_MOBILE_ADS_SDK_ROOT}'")
    else()
        list(APPEND GOOGLE_MOBILE_ADS_FRAMEWORKS "${GOOGLE_MOBILE_ADS_SDK_ROOT}/${_fw}.xcframework")
    endif()
endforeach()

#Check for optional frameworks — warn but don't fail if missing
foreach(_fw ${_GAD_OPTIONAL_FRAMEWORKS})
    if(EXISTS "${GOOGLE_MOBILE_ADS_SDK_ROOT}/${_fw}.xcframework")
        list(APPEND GOOGLE_MOBILE_ADS_FRAMEWORKS "${GOOGLE_MOBILE_ADS_SDK_ROOT}/${_fw}.xcframework")
    endif()
endforeach()

set(GOOGLE_MOBILE_ADS_FOUND FALSE)
if(_gad_all_required_found AND NOT "${GOOGLE_MOBILE_ADS_SDK_ROOT}" STREQUAL "")
    set(GOOGLE_MOBILE_ADS_FOUND TRUE)
endif()

if(GOOGLE_MOBILE_ADS_FOUND)
    if(NOT GOOGLE_MOBILE_ADS_FIND_QUIETLY)
        message(STATUS "Found GoogleMobileAds: ${GOOGLE_MOBILE_ADS_SDK_ROOT}")
        message(STATUS "GoogleMobileAds frameworks: ${GOOGLE_MOBILE_ADS_FRAMEWORKS}")
    endif()
else()
    if(GOOGLE_MOBILE_ADS_FIND_REQUIRED)
        message(FATAL_ERROR
            "Could not find the Google Mobile Ads SDK.\n"
            "Download the iOS SDK zip from https://developers.google.com/admob/ios/download\n"
            "Extract it and set GOOGLE_MOBILE_ADS_SDK_ROOT to the resulting folder.\n"
            "Expected to find at least: \${GOOGLE_MOBILE_ADS_SDK_ROOT}/GoogleMobileAds.xcframework"
        )
    endif()
endif()

mark_as_advanced(FORCE
    GOOGLE_MOBILE_ADS_FRAMEWORKS
)
