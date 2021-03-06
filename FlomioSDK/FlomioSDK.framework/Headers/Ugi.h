//
//  Ugi.h
//
//  Copyright (c) 2012 U Grok It. All rights reserved.
//

#define __UGI_H

#import <UIKit/UIKit.h>

#import "UgiConfigurationDelegate.h"
#import "UgiInventory.h"
#import "UgiRfidConfiguration.h"
#import "UgiEpc.h"
#import "UgiTag.h"
#import "UgiTagReadState.h"
#import "UgiInventoryDelegate.h"

///////////////////////////////////////////////////////////////////////////////////////
// Enums must be defined outside the class for Swift, but inside the class for
// Doxygen (the documentation generator)
///////////////////////////////////////////////////////////////////////////////////////

typedef NS_ENUM(int, UgiConnectionStates) {
  UGI_CONNECTION_STATE_NOT_CONNECTED,
  UGI_CONNECTION_STATE_CONNECTING,
  UGI_CONNECTION_STATE_INCOMPATIBLE_READER,
  UGI_CONNECTION_STATE_CONNECTED
};

typedef NS_ENUM(int, UgiReaderHardwareTypes) {
  UGI_READER_HARDWARE_UNKNOWN,
  UGI_READER_HARDWARE_GROKKER_1=5
};

typedef NS_ENUM(int, UgiOngoingOperations) {
  UGI_ONGOING_OPERATION_INACTIVE,
  UGI_ONGOING_OPERATION_INVENTORY,
  UGI_ONGOING_OPERATION_FIRMWARE_UPDATE,
  UGI_ONGOING_OPERATION_OTHER
};

typedef NS_ENUM(int, UgiPlaySoundSoundTypes) {
  UGI_PLAY_SOUND_FOUND_LAST
};

typedef NS_OPTIONS(NSUInteger, UgiLoggingTypes) {
  UGI_LOGGING_INTERNAL_BYTE_PROTOCOL = 0x1,
  UGI_LOGGING_INTERNAL_CONNECTION_ERRORS = 0x2,
  UGI_LOGGING_INTERNAL_CONNECTION_STATE = 0x4,
  UGI_LOGGING_INTERNAL_PACKET_PROTOCOL = 0x8,
  UGI_LOGGING_INTERNAL_COMMAND = 0x10,
  UGI_LOGGING_INTERNAL_INVENTORY = 0x20,
  UGI_LOGGING_INTERNAL_FIRMWARE_UPDATE = 0x40,
  UGI_LOGGING_INTERNAL_BATTERY_STATUS = 0x80,
  UGI_LOGGING_INTERNAL_PLATFORM_SPECIFIC = 0x100,
  UGI_LOGGING_STATE = 0x1000,
  UGI_LOGGING_INVENTORY = 0x2000,
  UGI_LOGGING_INVENTORY_DETAIL = 0x4000,
  UGI_LOGGING_BATTERY_STATUS = 0x8000
};

typedef NS_ENUM(NSUInteger, UgiGrokkerPasswordReturnValues) {
  UGI_GROKKER_PASSWORD_SUCCESS=0,
  UGI_GROKKER_PASSWORD_NOT_AUTHENTICATED=1,
  UGI_GROKKER_PASSWORD_WRONG_PASSWORD=2,
  UGI_GROKKER_PASSWORD_NO_PASSWORD_SET=3,
  UGI_GROKKER_PASSWORD_NOT_CONNECTED=4,
  UGI_GROKKER_PASSWORD_OTHER_ERROR=5
};

///////////////////////////////////////////////////////////////////////////////////////
// End enums
///////////////////////////////////////////////////////////////////////////////////////

/**
 Singleton class that implements the U Grok It API.

 The Ugi class is used as a singleton - only one instance of the class exists.

 The singleton object should be explicitly created by calling createSingleton.
 */
@interface Ugi : NSObject

/**
 Generic completion type
 */
typedef void (^VoidBlock)(void);

///////////////////////////////////////////////////////////////////////////////////////
#pragma mark - Lifecycle
///////////////////////////////////////////////////////////////////////////////////////

//! @cond
// YES if running iOS7 or later -- DEPRECATED because versions before iOS8 are no longer supported
+ (BOOL) isIOS7orLater __attribute__((deprecated));
// YES is running iOS8 or later -- DEPRECATED because versions before iOS8 are no longer supported
+ (BOOL) isIOS8orLater __attribute__((deprecated));
//! @endcond

//! Delegate to handle configuration events.
@property (readonly, nonnull) NSObject<UgiConfigurationDelegate> *configurationDelegate;

/**
 Create the singleton object with the default configuration handler,
 This is usually done in your application delegate's didFinishLaunchingWithOptions: method or in main.c
 */
+ (void) createSingleton;

/**
 Create the singleton object with a specific configuration handler
 This is usually done in your application delegate's didFinishLaunchingWithOptions: method or in main.c
 */
+ (void) createSingletonWithConfigurationDelegate:(nullable NSObject<UgiConfigurationDelegate> *)configurationDelegate;

/////////////

/**
 Get the singleton object.

 @return The one and only Ugi object, through which the application accesses the API.
 */
+ (Ugi  * _Nonnull) singleton;

/**
 Release the singleton object.

 Normally called in applicationWillTerminate
 */
+ (void) releaseSingleton;

/**
 See if the application has microphone permission.
 This always returns YES prior to iOS7
 @param completion Block called with permission status
 */
- (void) checkMicPermission:(nonnull void(^)(BOOL havePermission))completion;

/**
 See if this iOS device is a tablet
 @return isTablet    YES if this iOS device is a tablet
 */
+ (BOOL) isTablet;

/**
 Set the isTablet property. This can be useful for debugging.
 @param isTablet    value to set
 */
+ (void) debug_setIsTablet:(BOOL)isTablet;

//! @cond
- (UIInterfaceOrientationMask) desiredInterfaceOrientationMask __attribute__((deprecated));
// Get the preferred orientation (iOS6 and above), allow rotationm on tablets
// Obsolete - replaced with supportedInterfaceOrientationsWithAllowRotationOnTablet:
- (UIInterfaceOrientationMask) supportedInterfaceOrientationsAllowRotationOnTablet __attribute__((deprecated));
//! @endcond

/**
 Get the preferred orientation
 @param allowRotationOnTablet  YES to allow rotation on tablets
 @return                       UIInterfaceOrientationMask to return in supportedInterfaceOrientations method
 */
- (UIInterfaceOrientationMask) supportedInterfaceOrientationsWithAllowRotationOnTablet:(BOOL)allowRotationOnTablet;

///@name Connection

/**
 Try to open a connection to the reader.

 openConnection returns immediately, it does not wait for a connection to the reader
 to actually be established. If a reader is connected, the connection takes 400-500ms
 (just under half a second) for the connection sequence. Your app can get notification
 of connection state changes by registering with the default NotificationCenter with
 the name [Ugi singleton].NOTIFICAION_NAME_CONNECTION_STATE_CHANGED:
 @code
 [[NSNotificationCenter defaultCenter] addObserver:self
 selector:@selector(connectionStateChanged:)
 name:[Ugi singleton].NOTIFICAION_NAME_CONNECTION_STATE_CHANGED
 object:nil];
 @endcode

 This method is normally called in applicationDidBecomeActive but may be called
 elsewhere if the app does not want to always be connected to the reader.
 */
- (void) openConnection;

/**
 Close connection to the reader.

 This method is normally called in applicationWillTerminate.
 */
- (void) closeConnection;

///@}

///@name Connection Properties

//! Has openConnection has been called (without a corresponding call to closeConnecion)
@property (readonly, nonatomic) BOOL inOpenConnection;

//! Is anything is plugged into the audio jack (as best we can determine)
@property (readonly, nonatomic) BOOL isAnythingPluggedIntoAudioJack;

- (nonnull id) addConnectionStateListener:(nonnull VoidBlock)callback;

- (void) removeConnectionStateListener:(nonnull id)idFromAddConnectionStateListener;

///@}

///////////////////////////////////////////////////////////////////////////////////////
#pragma mark - Connection state
///////////////////////////////////////////////////////////////////////////////////////

///@name Connection Types

#if DOXYGEN   // Defined before class for Swift compatibility, documented here for Doxygen compatibility
/**
 States for the connection, sent with [Ugi singleton].NOTIFICAION_NAME_CONNECTION_STATE_CHANGED
 */
typedef enum {
  UGI_CONNECTION_STATE_NOT_CONNECTED,        //!< Nothing connected to audio port
  UGI_CONNECTION_STATE_CONNECTING,           //!< Something connected to audio port, trying to connect
  UGI_CONNECTION_STATE_INCOMPATIBLE_READER,  //!< Connected to an reader with incompatible firmware
  UGI_CONNECTION_STATE_CONNECTED             //!< Connected to reader
} UgiConnectionStates;
#endif

///@}

///@name Connection Properties

/**
 Notification of connection state changed is sent to default NSNotificationCenter. The object
 sent with the notification is an NSNumber containing the connection state.
 */
@property (readonly, nonatomic, nonnull) NSString *NOTIFICAION_NAME_CONNECTION_STATE_CHANGED;
/**
 Notification of connection state changed is sent to default NSNotificationCenter. The object
 sent with the notification is an NSNumber containing the connection state.
 */
@property (readonly, nonatomic, nonnull) NSString *NOTIFICAION_NAME_INVENTORY_STATE_CHANGED;

//! The current connection state
@property (readonly, nonatomic) UgiConnectionStates connectionState;

//! Whether the reader is connected (returns YES if connectionState == UGI_CONNECTION_STATE_CONNECTED)
@property (readonly, nonatomic) BOOL isConnected;

///@}

///////////////////////////////////////////////////////////////////////////////////////
#pragma mark - Grokker password
///////////////////////////////////////////////////////////////////////////////////////

///@name Grokker password

#if DOXYGEN   // Defined before class for Swift compatibility, documented here for Doxygen compatibility
/**
 Values returned by setGrokkerPassword and authenticateGrokker
 */
typedef enum {
  UGI_GROKKER_PASSWORD_SUCCESS=0,               //!< Successful
  UGI_GROKKER_PASSWORD_NOT_AUTHENTICATED=1,     //!< Failure, grokker is not authenticated
  UGI_GROKKER_PASSWORD_WRONG_PASSWORD=2,        //!< Incorrect password passed
  UGI_GROKKER_PASSWORD_NO_PASSWORD_SET=3,       //!< No password is set, so can't authenticate
  UGI_GROKKER_PASSWORD_NOT_CONNECTED=4,         //!< Grokker is not connected
  UGI_GROKKER_PASSWORD_OTHER_ERROR=5
} UgiGrokkerPasswordReturnValues;
#endif

//! YES if the Grokker requires authentication
@property (readonly, nonatomic) BOOL grokkerAuthenticationRequired;

//! YES if the Grokker has a password
@property (readonly, nonatomic) BOOL grokkerPasswordExists;

/**
 Set the Grokker's password. if a password is set, then the Grokker must be authenticated before
 any operation that changes tag data.

 @param password       Password to set
 @param completion     Code to run when operation is completed
 */
- (void) setGrokkerPassword:(NSString * _Nullable)password
             withCompletion:(nonnull void(^)(UgiGrokkerPasswordReturnValues result))completion;

/**
 Authenticate the Grokker.

 @param password       Password to authenticate with
 @param completion     Code to run when operation is completed
 */
- (void) authenticateGrokker:(NSString * _Nonnull)password
              withCompletion:(nonnull void(^)(UgiGrokkerPasswordReturnValues result))completion;

///@}

///////////////////////////////////////////////////////////////////////////////////////
#pragma mark - Inventory
///////////////////////////////////////////////////////////////////////////////////////

///@name Inventory

/**
 Start running inventory (if a reader is connected).

 If one or more EPCs are passed in, only they will be reported back to the delegate
 If no EPCs are passed (epcs=nil, numEpcs=0) then all EPCs will be reported back to the delegate

 If a small number of EPCs are passed (<=maxEpcsSentToReader), filtering is done on the
 reader and the reader plays sounds immediately (with no host interaction).
 Otherwise filtering is done on the host, and the host tells the reder when to play sounds
 which is slower.

 The inventory code keeps a history for each tag. This history is the number of finds for each
 time interval. The default is to store history for 20 intervals of 500ms each. This default can
 be modified via properties: historyIntervalMSec and historyDepth.

 @param delegate       Delegate object to report back to
 @param configuration  Configuration to use
 @param epcs           EPCs to find, all other EPCs are ignored (or nil to find all EPCs)
 @return               UgiInventory object that will hold the results of this inventory
 */
- (UgiInventory * _Nonnull) startInventory:(nonnull id<UgiInventoryDelegate>)delegate
                         withConfiguration:(UgiRfidConfiguration * _Nonnull)configuration
                                  withEpcs:(NSArray<UgiEpc *> * _Nonnull)epcs;

/**
 Start running inventory (if a reader is connected).

 If one or more EPCs are passed in, ignore these EPCs

 @param delegate       Delegate object to report back to
 @param configuration  Configuration to use
 @param epcsToIgnore   EPCs to ignore
 @return               UgiInventory object that will hold the results of this inventory
 */
- (UgiInventory * _Nonnull) startInventoryIgnoringEpcs:(nonnull id<UgiInventoryDelegate>)delegate
                                     withConfiguration:(UgiRfidConfiguration * _Nonnull)configuration
                                      withEpcsToIgnore:(NSArray<UgiEpc *> * _Nonnull)epcsToIgnore;

/**
 Start running inventory to find any tags

 @param delegate   Delegate object to report back to
 @param configuration  Configuration to use
 @return               UgiInventory object that will hold the results of this inventory
 */
- (UgiInventory * _Nonnull) startInventory:(nonnull id<UgiInventoryDelegate>)delegate
                         withConfiguration:(UgiRfidConfiguration * _Nonnull)configuration;

/**
 Start running inventory to find one specific tag

 @param delegate   Delegate object to report back to
 @param configuration  Configuration to use
 @param epc            EPC to find, all other EPCs are ignored
 @return               UgiInventory object that will hold the results of this inventory
 */
- (UgiInventory * _Nonnull) startInventory:(nonnull id<UgiInventoryDelegate>)delegate
                         withConfiguration:(UgiRfidConfiguration * _Nonnull)configuration
                                   withEpc:(UgiEpc * _Nonnull)epc;

//! Get the currently active inventory object (if any)
@property (readonly, nonatomic, nullable) UgiInventory * activeInventory;

- (nonnull id) addInventoryStateListener:(nonnull VoidBlock)callback;

- (void) removeInventoryStateListener:(nonnull id)idFromAddInventoryStateListener;

///////////////////////////////////////////////////////////////////////////////////////
#pragma mark - Reader info
///////////////////////////////////////////////////////////////////////////////////////

///@name Reader Information Properties

//! Protocol version that the host requires
@property (readonly, nonatomic) int requiredProtocolVersion;

//! Protocol version that the host supports
@property (readonly, nonatomic) int supportedProtocolVersion;

//////////////

//
// These values are not valid until the reader is connected
//

//! Reader protocol version
@property (readonly, nonatomic) int readerProtocolVersion;

///@}

///@name Reader Information Types

/**
 Reader hardware type and version
 */
#if DOXYGEN   // Defined before class for Swift compatibility, documented here for Doxygen compatibility
typedef enum {
  UGI_READER_HARDWARE_UNKNOWN,                  //!< Unknown
  UGI_READER_HARDWARE_GROKKER_1=5               //!< The original Grokker
} UgiReaderHardwareTypes;
#endif

///@}

///@name Reader Information Properties

//! Reader's model
@property (readonly, nonatomic, nullable) NSString *readerHardwareModel;

//! Reader's hardware type
@property (readonly, nonatomic) UgiReaderHardwareTypes readerHardwareType;
//! Reader's hardware version
@property (readonly, nonatomic) int readerHardwareRevision;

//! Firmware version in the reader, major
@property (readonly, nonatomic) int firmwareVersionMajor;
//! Firmware version in the reader, minor
@property (readonly, nonatomic) int firmwareVersionMinor;
//! Firmware version in the reader, build
@property (readonly, nonatomic) int firmwareVersionBuild;

//! Reader's serial number
@property (readonly, nonatomic) int readerSerialNumber;

///@}

///@name Reader Information Properties

//! name of region of the world, not localized
@property (readonly, nonatomic, nullable) NSString *unlocalizedRegionName;

//! Maximum number of tones in a sound
@property (readonly, nonatomic) int maxTonesInSound;

//! Maximum power that the reader can use
@property (readonly, nonatomic) double maxPower;

//! Maximum sensitivity
@property (readonly, nonatomic) int maxSensitivity;

//! Maximum number of volume levels
@property (readonly, nonatomic) int numVolumeLevels;

//! YES if the reader has battery power
@property (readonly, nonatomic) BOOL hasBattery;

//! YES if the reader has external power
@property (readonly, nonatomic) BOOL hasExternalPower;

//! Battery capacity in minutes
@property (readonly, nonatomic) int batteryCapacity;

//! Battery capacity in mAh
@property (readonly, nonatomic) int batteryCapacity_mAh;

//! YES if device initialized successfully
@property (readonly, nonatomic) BOOL deviceInitializedSuccessfully;


//! Description of the reader, generally used for debugging
@property (readonly, nonatomic, nullable) NSString *readerDescription;

///@}


///////////////////////////////////////////////////////////////////////////////////////
#pragma mark - Localization
///////////////////////////////////////////////////////////////////////////////////////

/**
 Get the localized region name for an unlocalized region name

 @param unlocalizedRegionName  unlocalized region name
 @return                       localized region name
 */
- (NSString * _Nonnull) localizedRegionNameForName:(NSString * _Nonnull)unlocalizedRegionName;

///////////////////////////////////////////////////////////////////////////////////////
#pragma mark - Reader Configuration: Sounds
///////////////////////////////////////////////////////////////////////////////////////

/**
 @public
 Geiger counter sound configuration, used by getGeigerCounterSound and setGeigerCounterSound
 */
typedef struct {
  int frequency;          //!< @public Frequency for each click (Hz)
  int durationMsec;       //!< @public Duration of each click, in milliseconds
  double clickRate;        //!< @public Ratio used for translating finds/second into clicks/second
  int maxClicksPerSecond; //!< @public Maximum number of clicks per second
  int historyDepthMsec;   //!< @public Number of history periods to consider for determining click rate
} UgiGeigerCounterSound;

///@name Configuration

/**
 Get the current geiger counter sound configuration

 This configuration is used if UGI_INVENTORY_SOUNDS_GEIGER_COUNTER is
 passed to startInventory

 @param config  Buffer to fill
 @return        YES if successful, NO if reader has never been connected)
 */
- (BOOL) getGeigerCounterSound:(UgiGeigerCounterSound * _Nonnull)config;

/**
 Set the geiger counter sound configuration

 This configuration is used if UGI_INVENTORY_SOUNDS_GEIGER_COUNTER is
 passed to startInventory

 If no reader is connected, the reader will be configured with these parameters
 after a connection is established. Similiarly, if the reader is disconnected and
 reconncted, these parameters will be configured with these parameters.

 @param config    Configuration parameters to set
 */
- (void) setGeigerCounterSound:(UgiGeigerCounterSound * _Nonnull)config;

///@}

/**
 @public
 A single tone, used by setFoundItemSound and setFoundLastItemSound
 */
typedef struct {
  int frequency;      //!< @public Frequency of tone (Hz)
  int durationMsec;   //!< @public Duration of tone, in milliseconds
} UgiSpeakerTone;

///@name Configuration

/**
 Get the current set of tones played when an item is found

 This sound is used if UGI_INVENTORY_SOUNDS_FIRST_FIND or
 UGI_INVENTORY_SOUNDS_FIRST_FIND_AND_LAST is passed to startInventory

 @return        A memory buffer containing an array of UgiSpeakerTone
 structures, ending in a structure with durationMsec==0.
 The caller must free() this bufffer.
 Returns NULL if a reader has never been connected
 */
- (UgiSpeakerTone * _Nonnull) getFoundItemSound;

/**
 Set the set of tones played when an item is found

 This sound is used if UGI_INVENTORY_SOUNDS_FIRST_FIND or
 UGI_INVENTORY_SOUNDS_FIRST_FIND_AND_LAST is passed to startInventory

 If no reader is connected, the reader will be configured with these parameters
 after a connection is established. Similiarly, if the reader is disconnected and
 reconncted, these parameters will be configured with these parameters.

 @param sound     Array of UgiSpeakerTone structures, ending in a
 structure with durationMsec==0.
 */
- (void) setFoundItemSound:(UgiSpeakerTone * _Nonnull)sound;

/**
 Get the current set of tones played when the last item is found

 This sound is used if UGI_INVENTORY_SOUNDS_FIRST_FIND_AND_LAST
 is passed to startInventory

 @return        A memory buffer containing an array of UgiSpeakerTone
 structures, ending in a structure with durationMsec==0.
 The caller must free() this bufffer.
 Returns NULL if a reader has never been connected
 */
- (UgiSpeakerTone * _Nonnull) getFoundLastItemSound;

/**
 Set the set of tones played when the last item is found

 This sound is used if UGI_INVENTORY_SOUNDS_FIRST_FIND_AND_LAST
 is passed to startInventory

 If no reader is connected, the reader will be configured with these parameters
 after a connection is established. Similiarly, if the reader is disconnected and
 reconncted, these parameters will be configured with these parameters.

 @param sound     Array of UgiSpeakerTone structures, ending in a
 structure with durationMsec==0.
 */
- (void) setFoundLastItemSound:(UgiSpeakerTone * _Nonnull)sound;

///@}

///////////////////////////////////////////////////////////////////////////////////////
#pragma mark - Diagnostic data
///////////////////////////////////////////////////////////////////////////////////////

/**
 @public
 Diagnostic data
 */
typedef struct {
  double byteProtocolSkewFactor;           //!< @public Factor the the reader's clock is off by
  int byteProtocolBytesSent;               //!< @public Bytes sent by reader
  int byteProtocolBytesReceived;           //!< @public Bytes received by reader
  int byteProtocolSubsequentReadTimeouts;  //!< @public Reader timeouts waiting for a next byte in a packet
  int packetProtocolPacketsSent;           //!< @public Packets sent by reader
  int packetProtocolPacketsReceived;       //!< @public Bytes received by reader
  int packetProtocolSendFailures;          //!< @public Reader failures sending a packet
  int packetProtocolSendRetries;           //!< @public Reader retries sending packets
  int packetProtocolSendTimeouts;          //!< @public Reader timeouts sending packets
  int packetProtocolInvalidPackets;        //!< @public Reader invalid packets received
  int packetProtocolInternalCrcMismatches; //!< @public Reader responses received with illegal enbedded CRCs
  int packetProtocolCrcMismatches;         //!< @public Reader packets received with wrong CRC
  int rawInventoryRounds;                  //!< @public Number of inventory rounds run
  int rawTagFinds;                         //!< @public Number of tag finds
  int inventoryUnique;                     //!< @public Number of unique tags found
  int inventoryForgotten;                  //!< @public Number of forgotten tags
  int inventoryForgottenNotAcknowledged;   //!< @public Number of forgotten tags not acknowledged by the host
  int inventoryForgottenNotSent;           //!< @public Number of forgotten tags not sent to the host
} UgiDiagnosticData;

///@name Diagnostic information

/**
 Get diagnostic data

 @param data            Buffer to fill
 @param reset           YES to reset counters
 @return        YES if successful, NO if reader has never been connected)
 */
- (BOOL) getDiagnosticData:(UgiDiagnosticData * _Nonnull)data
             resetCounters:(BOOL)reset;

///@}

///////////////////////////////////////////////////////////////////////////////////////
#pragma mark - Battery
///////////////////////////////////////////////////////////////////////////////////////

/**
 @public
 Battery data
 */
typedef struct _UgiBatteryInfo {
  BOOL canScan;                     //!< @public YES if battery is capable of scanning
  BOOL externalPowerIsConnected;    //!< @public YES if external power is connected
  BOOL isCharging;                  //!< @public YES if battery is charging
  BOOL temperatureAtOrAboveThreshold;         //!< @public YES if temperature is at or above the threshold for scanning
  BOOL temperatureWithin5DegreesOfThreshold;  //!< @public YES if temperature is within 5 degrees C of the threshold for scanning
  BOOL temperatureWithin10DegreesOfThreshold;  //!< @public YES if temperature is within 10 degrees C of the threshold for scanning
  BOOL temperatureWithin20DegreesOfThreshold;  //!< @public YES if temperature is within 20 degrees C of the threshold for scanning
  int minutesRemaining;             //!< @public Minutes of scanning remaining
  int percentRemaining;             //!< @public Percent of scanning time remaining. This is not very accurate while charging
  double voltage;                   //!< @public Battery voltage
} UgiBatteryInfo;

//! @cond
// DEPRECATED - use reportBatteryInfoPeriodically if possible
// Also used by the Xamarin wrapper
- (void) getBatteryInfoWithCompletion:(nonnull void(^)(UgiBatteryInfo * _Nonnull info))completion __attribute__((deprecated));
//! @endcond

//////

/**
 Operation used by ReportConnectionStateAndBatteryInfoCallback
 */
#if DOXYGEN   // Defined before class for Swift compatibility, documented here for Doxygen compatibility
typedef enum {
  UGI_ONGOING_OPERATION_INACTIVE,        //!< No operation active
  UGI_ONGOING_OPERATION_INVENTORY,       //!< Running inventory
  UGI_ONGOING_OPERATION_FIRMWARE_UPDATE, //!< Firmware update in progress
  UGI_ONGOING_OPERATION_OTHER            //!< Other operation in progress
} UgiOngoingOperations;
#endif

/**
 Callback function for reportBatteryInfoPeriodically

 @param connectionState            Connection state
 @param ongoingOperation           Ongoing operation
 @param batteryInformationIsValid  YES if externalPowerIsConnected, isCharging, canScan and averagedBatteryPercentage are valid
 @param externalPowerIsConnected   YES if external power is connected
 @param isCharging                 YES if battery is charging
 @param canScan                    YES if battery is capable of scanning
 @param canGetBatteryWhileScanning  YES if this Grokker can return battery information while scanning (firmware 1.11.2 or above)
 @param temperatureAtOrAboveThreshold           YES if temperature is at or above the threshold for scanning
 @param temperatureWithin5DegreesOfThreshold    YES if temperature is within 5 degrees C of the threshold for scanning
 @param temperatureWithin10DegreesOfThreshold   YES if temperature is within 10 degrees C of the threshold for scanning
 @param temperatureWithin20DegreesOfThreshold   YES if temperature is within 20 degrees C of the threshold for scanning
 @param averagedBatteryPercentage  Battery percentage averaged over a short period, so more stable, -1
 if not valid. Only valid if external power is not connected
 @param statusDescription          Text description
 */
typedef void (^ReportConnectionStateAndBatteryInfoCallback)(UgiConnectionStates connectionState,
                                                            UgiOngoingOperations ongoingOperation,
                                                            BOOL batteryInformationIsValid,
                                                            BOOL externalPowerIsConnected,
                                                            BOOL isCharging,
                                                            BOOL canScan,
                                                            BOOL canGetBatteryWhileScanning,
                                                            BOOL temperatureAtOrAboveThreshold,
                                                            BOOL temperatureWithin5DegreesOfThreshold,
                                                            BOOL temperatureWithin10DegreesOfThreshold,
                                                            BOOL temperatureWithin20DegreesOfThreshold,
                                                            int averagedBatteryPercentage,
                                                            NSString * _Nonnull statusDescription);

/**
 Periodically report connection state and battery information to the callback.

 @param callback    Callback function
 @return            id to pass to endReportConnectionStateAndBatteryInfoPeriodically
 */
- (nonnull id) reportConnectionStateAndBatteryInfoPeriodically:(nonnull ReportConnectionStateAndBatteryInfoCallback)callback;

/**
 End periodic connection state and battery info reports.

 @param idFromReportConnectionStateAndBatteryInfoPeriodically    id returned from reportConnectionStateAndBatteryInfoPeriodically
 */
- (void) endReportConnectionStateAndBatteryInfoPeriodically:(nonnull id)idFromReportConnectionStateAndBatteryInfoPeriodically;

/////////////

/**
 Set an adjustment to the battery percentage (the adjustment value is added to or
 subtracted from the real battery percentage). This is useful for development
 for testing displaying of different battery levels.
 @param adjustment    adjustment value
 @param completion    Code to run afterwards
 */
- (void) debug_setBatteryPercentageAdjustment:(int)adjustment
                               withCompletion:(nonnull void(^)(BOOL success))completion;

/**
 Set an adjustment to the battery temperature (the adjustment value is added to the
 real temperature). This is useful for development for testing hanlding of warnings
 when the Grokker is used in very hot environments.
 @param adjustment    adjustment value (must be positive or zero)
 @param completion    Code to run afterwards
 */
- (void) debug_setTemperatureAdjustment:(int)adjustment
                         withCompletion:(nonnull void(^)(BOOL success))completion;

/////////////

//
// Alternative version for Xamarin Wrapper
//
//! @cond
- (void) getBatteryInfoArrayWithCompletion:(nonnull void(^)(NSArray * _Nonnull raw))completion;
//! @endcond

///////////////////////////////////////////////////////////////////////////////////////
#pragma mark - Misc
///////////////////////////////////////////////////////////////////////////////////////

//! @cond
/**
 Sounds playable on the reader via playSound
 */
#if DOXYGEN   // Defined before class for Swift compatibility, documented here for Doxygen compatibility
typedef enum {
  UGI_PLAY_SOUND_FOUND_LAST     //!< The "last item found" sound
} UgiPlaySoundSoundTypes;
#endif

/**
 Play a sound on the reader

 @param sound   Sound to play
 */
- (void) playSound:(UgiPlaySoundSoundTypes)sound;
//! @endcond

///////////////////////////////////////////////////////////////////////////////////////
#pragma mark - Logging
///////////////////////////////////////////////////////////////////////////////////////

///@name Logging Types

/**
 Types of logging.

 The default is no logging. The internal logging types are primarily for debugging
 of the API itself.
 */
#if DOXYGEN   // Defined before class for Swift compatibility, documented here for Doxygen compatibility
typedef enum {
  UGI_LOGGING_INTERNAL_BYTE_PROTOCOL = 0x1,       //!< Lowest level communication protocol:
                                                  //!< connection handshaking and byte send/receive
  UGI_LOGGING_INTERNAL_CONNECTION_ERRORS = 0x2,   //!< Low level communication errors
  UGI_LOGGING_INTERNAL_CONNECTION_STATE = 0x4,    //!< Low level connection state changes
  UGI_LOGGING_INTERNAL_PACKET_PROTOCOL = 0x8,     //!< Packet send/receive
  UGI_LOGGING_INTERNAL_COMMAND = 0x10,            //!< Command send/receive
  UGI_LOGGING_INTERNAL_INVENTORY = 0x20,          //!< Low-level inventory
  UGI_LOGGING_INTERNAL_FIRMWARE_UPDATE = 0x40,    //!< Low-level firmware update
  UGI_LOGGING_INTERNAL_BATTERY_STATUS = 0x80,     //!< Low-level battery status reporting
  UGI_LOGGING_INTERNAL_PLATFORM_SPECIFIC = 0x100, //!< Low-level platform specific debugging

  UGI_LOGGING_STATE = 0x1000,             //!< Connection and inventory state
  UGI_LOGGING_INVENTORY = 0x2000,         //!< Inventory activity
  UGI_LOGGING_INVENTORY_DETAIL = 0x4000,  //!< Inventory details
  UGI_LOGGING_BATTERY_STATUS = 0x8000     //!< Battery status details
} UgiLoggingTypes;
#endif

///@}

///@name Logging Properties

/**
 The current logging status
 */
@property (nonatomic) UgiLoggingTypes loggingStatus;

/**
 If YES, add a timestamp to each logging line (default = NO)
 */
@property (nonatomic) BOOL loggingTimestamp;

typedef void (^UgiLoggingCallback)(NSString * _Nonnull string);

/**
 Set the logging callback

 By default logging goes to NSLog(), if a logging callback is set, it goes to this callback instead
 @param callback   Callback to send logging output to
 */
+ (void) setLoggingCallback:(nullable UgiLoggingCallback)callback;

/**
 Log a string, either to NSLog or to the logging callback
 */
extern void UgiLog(NSString * _Nonnull format, ...);

/**
 Log a string, either to NSLog or to the logging callback
 */
+ (void) log:(NSString * _Nonnull)s;

///@}

///////////////////////////////////////////////////////////////////////////////////////
#pragma mark - SDK version
///////////////////////////////////////////////////////////////////////////////////////

///@name SDK Version Properties

/**
 SDK Version, major

 Version 1.4.1 - September 10, 2014 - Reader protocol 17<br>
 RFID configuration changes<br>
 Bug fixes<br>
 <br>
 Version 1.5.1 - October 30, 2014 - Reader protocol 19<br>
 Bug fixes and performance improvements<br>
 <br>
 Version 1.6.1 - December 18, 2014 - Reader protocol 19<br>
 EU Grokker support (region setting)<br>
 <br>
 Version 1.7.2 - February 5, 2015 - Reader protocol 19<br>
 Streamlined UI for housekeeping tasks (set region and firmware update): added UgiConfigurationDelegate and UgiDefaultConfigurationUi<br>
 <br>
 Version 1.7.3 - February 11, 2015 - Reader protocol 19<br>
 Bug fix for app inactive/active while inventory is running<br>
 <br>
 Version 1.7.5 - March 9, 2015 - Reader protocol 19<br>
 Bug fix for passing both a select mask and EPCs to startInventory<br>
 <br>
 Version 1.7.6 - March 26, 2015 - Reader protocol 19<br>
 Better handling of protocol errors while starting/stopping inventory<br>
 <br>
 Version 1.7.7 - April 8, 2015 - Reader protocol 19<br>
 Minor bug fixes with setting region<br>
 <br>
 Version 1.7.8 - April 25, 2015 - Reader protocol 19<br>
 Bug fix for intermittent issue with pausing/resuming connection<br>
 <br>
 Version 1.7.13 - July 9, 2015 - Reader protocol 20<br>
 Updated Australia region information<br>
 Do not allow getting battery level during firmware update<br>
 <br>
 Version 1.7.16 - August 15, 2015 - Reader protocol 20<br>
 Fix crashing bug in SetRegion if no region selected<br>
 <br>
 Version 1.7.18 - September 19, 2015 - Reader protocol 20<br>
 iOS 9 / XCode 7<br>
 <br>
 Version 1.7.19 - September 24, 2015 - Reader protocol 20<br>
 Fix problem with setting region on new Grokkers<br>
 <br>
 Version 1.8.1 - October 5, 2015 - Reader protocol 20<br>
 API for controlling the SDK's Internet use<br>
 Fix problem with archive builds and bitcode<br>
 <br>
 Version 1.8.3 - October 20, 2015 - Reader protocol 20<br>
 Fix for iOS6 and initializing new Grokkers<br>
 Remove minimum EPC length<br>
 <br>
 Version 1.8.5 - October 28, 2015 - Reader protocol 20<br>
 Fix for accidentally hitting the dictation key while Grokker is connected<br>
 <br>
 Version 1.9.1 - January 13, 2016 - Reader protocol 21<br>
 Protocol 21: force-tari-25 flag, delay-after-select flag<br>
 API for reading RF Micron Magnus sensor tags<br>
 Fix DetailedPerReadData readData1 and readData2 to not be byte-swizzled<br>
 Added Ugi requiresFirmwareVersion method<br>
 <br>
 Version 1.9.5 - March 5, 2016 - Reader protocol 21<br>
 Added reportConnectionStateAndBatteryInfoPeriodically:<br>
 <br>
 Version 1.9.6 - March 14, 2016 - Reader protocol 21<br>
 Bug fix with reportConnectionStateAndBatteryInfoPeriodically:<br>
 <br>
 Version 1.10.1 - April 2, 2016 - Reader protocol 21<br>
 UgiUiUtil rework, added UI building blocks<br>
 <br>
 Version 1.11.1 - May 20, 2016 - Reader protocol 21<br>
 More work on UI building blocks<br>
 Added Swift support<br>
 <br>
 Version 1.11.2 - May 31, 2016 - Reader protocol 21<br>
 Farsens tag support<br>
 API Demo bug fix<br>
 Minor UgiUiUtil API enhancement<br>
 <br>
 Version 1.12.1 - July 11, 2016 - Reader protocol 22<br>
 Removed support for protocol levels before 17<br>
 Protocol 22: battery info while scanning, temperature in reportConnectionStateAndBatteryInfoPeriodically<br>
 <br>
 Version 1.12.2 - July 29, 2016 - Reader protocol 22<br>
 Battery status images incorporating warning about Grokker being too hot<br>
 <br>
 Version 1.12.3 - August 11, 2016 - Reader protocol 22<br>
 Audio protocol fix to better handle an obscure error condition<br>
 <br>
 Version 1.13.1 - September 8, 2016 - Reader protocol 23<br>
 Add localization support<br>
 Fix UgiTitleView to center the title better<br>
 <br>
 Version 1.13.2 - September 28, 2016 - Reader protocol 23<br>
 Small API addition to mirror Android<br>
 Small bug fixes<br>
 <br>
 Version 1.13.3 - October 28, 2016 - Reader protocol 23<br>
 Check battery level before doing firmware update<br>
 Small translation fixes<br>
 Small change to battery status images<br>
 <br>
 Version 1.13.4 - November 23, 2016 - Reader protocol 23<br>
 Fix race condition bug in reporting battery info with very rapid disconnect/connect<br>
 Fix bug writing a data and passing data==previousData<br>
 Swift 3.0 compatibility<br>
 <br>
 Version 1.14.1 - December 15, 2016 - Reader protocol 24<br>
 Support a password in readTag (requires firmware 1.12.3)<br>
 Support for killTag (requires firmware 1.12.3)<br>
 Fix objc_autoreleaseNoPool warning>br>
 Added UGI_LOGGING_BATTERY_STATUS, UGI_LOGGING_INTERNAL_BATTERY_STATUS and UGI_LOGGING_INTERNAL_PLATFORM_SPECIFIC<br>
 Add UgiLog(), improve app's ability to capture logging<br>
 <br>
 Version 1.15.1 - January 25, 2017 - Reader protocol 26<br>
 Support for authentication (requires firmware 1.12.4)<br>
 Single find mode (requires firmware 1.12.5)<br>
 <br>
 Version 1.15.3 - February 8, 2017 - Reader protocol 26<br>
 Fix bug in 64-bit devices and per-read data<br>
 <br>
 Version 1.15.4 - February 15, 2017 - Reader protocol 26<br>
 Small localization changes<br>
 <br>
 Version 1.15.6 - March 31, 2017 - Reader protocol 26<br>
 XCode 8.3 support<br>
 <br>
 Version 1.16.1 - July 14, 2017 - Reader protocol 26<br>
 Set Region removed (will be done in the factory)<br>
 Use Turck part number/model<br>
 Change internal Reachability symbols to avoid conflicts<br>
 Fixed a couple of minor memory leaks<br>
 <br>
 Version 1.16.2 - September 27, 2017 - Reader protocol 26<br>
 Fix minor memory leak<br>
 Minor region table changes<br>
 Reduce MAX_RFID_CONFIGURATION_MASK_LENGTH_BYTES to 12, enforce this<br>
 Support for China's frequencies<br>
 Support for iOS11 and XCode 9, drop support for iOS6 and iOS7<br>
 Version 1.16.4 - October 13, 2017 - Reader protocol 26<br>
 Fix SINGLE_FIND mode, broken in previous SDK<br>
 */
@property (readonly, nonatomic) int sdkVersionMajor;
//! SDK Version, minor
@property (readonly, nonatomic) int sdkVersionMinor;
//! SDK Version, minor
@property (readonly, nonatomic) int sdkVersionBuild;
//! SDK Version, date/time
@property (readonly, nonatomic, nonnull) NSDate *sdkVersionDateTime;

///@}

@end
