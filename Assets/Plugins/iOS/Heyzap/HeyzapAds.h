/*
 * Copyright (c) 2015, Heyzap, Inc.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 * * Redistributions of source code must retain the above copyright
 *   notice, this list of conditions and the following disclaimer.
 *
 * * Redistributions in binary form must reproduce the above copyright
 *   notice, this list of conditions and the following disclaimer in the
 *   documentation and/or other materials provided with the distribution.
 *
 * * Neither the name of 'Heyzap, Inc.' nor the names of its contributors
 *   may be used to endorse or promote products derived from this software
 *   without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
 * TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */


#import <Foundation/Foundation.h>
#import <UIKit/UIKit.h>
#import "HZLog.h"
#import "HZInterstitialAd.h"
#import "HZIncentivizedAd.h"
#import "HZBannerAdController.h"
#import "HZBannerAdOptions.h"
#import "HZShowOptions.h"
#import "HZDemographics.h"
#import "HZVideoAd.h"
#import "FYBOfferWallViewController.h"
#import "FYBRequestParameters.h"
#import "FYBVirtualCurrencyClient.h"

#ifndef NS_ENUM
#define NS_ENUM(_type, _name) enum _name : _type _name; enum _name : _type
#endif

#define SDK_VERSION @"10.2.2"

#if __has_feature(objc_modules)
@import AdSupport;
@import CoreGraphics;
@import CoreTelephony;
@import MediaPlayer;
@import QuartzCore;
@import StoreKit;
@import iAd;
@import MobileCoreServices;
@import Security;
@import SystemConfiguration;
@import EventKit;
@import EventKitUI;
@import MessageUI;
@import CoreLocation;
#endif

typedef NS_ENUM(NSUInteger, HZAdOptions) {
    HZAdOptionsNone = 0 << 0, // 0
    /**
     *  Pass this to disable automatic prefetching of ads. Ad prefetching occurs immediately after you initialize the Heyzap SDK and also after ads are dismissed.
     */
    HZAdOptionsDisableAutoPrefetching = 1 << 0, // 1
    /**
     *  Pass this if you are only integrating the Heyzap SDK into your app to track game installs as an advertiser. No ads will be fetched.
     */
    HZAdOptionsInstallTrackingOnly = 1 << 1, // 2
    /**
     *  @deprecated
     *  Please use HZAdOptionsInstallTrackingOnly instead.
     */
    HZAdOptionsAdvertiserOnly DEPRECATED_ATTRIBUTE = HZAdOptionsInstallTrackingOnly,
    // This doesn't do anything for iOS, but is here to keep parity with the Android SDK's flag values for the sake of Unity, AIR, etc.
    HZAdOptionsAmazon DEPRECATED_ATTRIBUTE = 1 << 2, // 4
    /**
     *  Pass this to disable mediation. This is not required, but is recommended for developers not using mediation (i.e: not integrating any 3rd-pary network SDKs). If you're mediating Heyzap through someone (e.g. AdMob), it is *strongly* recommended that you disable Heyzap's mediation to prevent any potential conflicts.
     */
    HZAdOptionsDisableMedation = 1 << 3, // 8
    /**
     *  Pass this to disable recording of In-App Purchase data
     */
    HZAdOptionsDisableAutomaticIAPRecording = 1 << 4, // 16
    
    //placeholder for android flag value NATIVE_ADS_ONLY = 1 << 5 // 32
    // (iOS does not (yet) use this option, but iOS and Android need to keep the same flag values for the sake of Unity, AIR, etc.)
    
    /**
     *  Pass this flag to mark mediated ads as "child-directed". This value will be passed on to networks that support sending such an option (for purposes of the Children's Online Privacy Protection Act (COPPA)).
     *  Currently, only AdMob is passed this information (see https://developers.google.com/admob/ios/targeting#child-directed_setting ). The AdMob setting will be left alone if this flag is not passed when the Heyzap SDK is started.
     */
    HZAdOptionsChildDirectedAds = 1 << 6, // 64
};

// HZAdsDelegate Callback NSNotifications
extern NSString * const HZMediationDidShowAdNotification;
extern NSString * const HZMediationDidFailToShowAdNotification;
extern NSString * const HZMediationDidReceiveAdNotification;
extern NSString * const HZMediationDidFailToReceiveAdNotification;
extern NSString * const HZMediationDidClickAdNotification;
extern NSString * const HZMediationDidHideAdNotification;
extern NSString * const HZMediationWillStartAdAudioNotification;
extern NSString * const HZMediationDidFinishAdAudioNotification;
// HZIncentivizedAdDelegate Callback NSNotifications
extern NSString * const HZMediationDidCompleteIncentivizedAdNotification;
extern NSString * const HZMediationDidFailToCompleteIncentivizedAdNotification;

// NSNotifications
extern NSString * const HZRemoteDataRefreshedNotification __attribute__((unavailable("This feature is not available in SDK 10; use SDK 9 if you need it.")));
extern NSString * const HZMediationNetworkCallbackNotification __attribute__((unavailable("This feature is not available in SDK 10; use SDK 9 if you need it.")));


/** The `HZAdsDelegate` protocol provides global information about our ads. If you want to know if we had an ad to show after calling `showAd` (for example, to fallback to another ads provider). It is recommend using the `showAd:completion:` method instead. */
@protocol HZAdsDelegate<NSObject>

@optional

#pragma mark - Showing ads callbacks

/**
 *  Called when we succesfully show an ad.
 *
 *  @param tag The identifier for the ad.
 */
- (void)didShowAdWithTag: (NSString *) tag;

/**
 *  Called when an ad fails to show
 *
 *  @param tag   The identifier for the ad.
 *  @param error An NSError describing the error
 */
- (void)didFailToShowAdWithTag: (NSString *) tag andError: (NSError *)error;

/**
 *  Called when a valid ad is received
 *
 *  @param tag The identifier for the ad.
 */
- (void)didReceiveAdWithTag: (NSString *) tag;

/**
 *  Called when our server fails to send a valid ad, like when there is a 500 error.
 *
 *  @param tag The identifier for the ad.
 */
- (void)didFailToReceiveAdWithTag: (NSString *) tag;



// Should probably have new API: didFailToReceiveAd (no tag)
// didRecieveAd (no tag)


/**
 *  Called when the user clicks on an ad.
 *
 *  @param tag An identifier for the ad.
 */
- (void)didClickAdWithTag: (NSString *) tag DEPRECATED_MSG_ATTRIBUTE("Click callbacks are no longer supported. Many networks do not report clicks, making this callback inherently inaccurate.");
/**
 *  Called when the ad is dismissed.
 *
 *  @param tag An identifier for the ad.
 */
- (void)didHideAdWithTag: (NSString *) tag;

/**
 *  Called when an ad will use audio
 */
- (void)willStartAudio;

/**
 *  Called when an ad will finish using audio
 */
- (void) didFinishAudio;

@end

/** The HZIncentivizedAdDelegate protocol provides global information about using an incentivized ad. If you want to give the user a reward
 after successfully finishing an incentivized ad, implement the didCompleteAd method */
@protocol HZIncentivizedAdDelegate<HZAdsDelegate>

@optional

/** Called when a user successfully completes viewing an ad */
- (void)didCompleteAdWithTag: (NSString *) tag;
/** Called when a user does not complete the viewing of an ad */
- (void)didFailToCompleteAdWithTag: (NSString *) tag;

@end

/**
 *  A class with miscellaneous Heyzap Ads methods. All methods on this class must be called from the main queue.
 */
@interface HeyzapAds : NSObject

+ (void) startWithPublisherID: (NSString *) publisherID andOptions: (HZAdOptions) options;
+ (void) startWithPublisherID:(NSString *)publisherID andOptions:(HZAdOptions)options andFramework: (NSString *) framework;
+ (void) startWithPublisherID: (NSString *) publisherID;

+ (BOOL) isStarted;
+ (void) setDebugLevel:(HZDebugLevel)debugLevel;
+ (void) setDebug:(BOOL)choice;
+ (void) setOptions: (HZAdOptions)options;
+ (void) setFramework: (NSString *) framework;
+ (void) setMediator: (NSString *) mediator;

/**
 *  Heyzap uses your app's bundle identifier to lookup your game in our database. By default, we lookup the bundle identifier from your Info.plist file.
 *
 *  If you need to use a different bundle identifier to identify your app than the one in the Info.plist file, you can call this method to override the bundle ID Heyzap uses. This supports use cases like having a different bundle ID in your Info.plist for production and development builds.
 *
 * You must call this method before starting the SDK.
 *
 *  @param bundleIdentifier The bundle identifier Heyzap should use to lookup your game in our database.
 *
 *  @exception NSInternalInconsistencyException is thrown if this method is called after starting the SDK.
 *  @exception NSInternalInconsistencyException if bundleIdentifier is `nil`.
 */
+ (void)setBundleIdentifier:(NSString *)bundleIdentifier;
+ (NSString *) defaultTagName;

/**
 * Returns a dictionary of developer-settable data or an empty dictionary if no data is available.
 *
 * @note This data is cached, so it will usually be available at app launch. It is updated via a network call that is made when `[HeyzapAds startWithPublisherId:]` (or one of its related methods) is called. If you want to guarantee that the data has been refreshed, only use it after receiving an NSNotification with name=`HZRemoteDataRefreshedNotification`. The userInfo passed with the notification will be the same NSDictionary you can receive with this method call.
 */
+ (NSDictionary *) remoteData __attribute__((unavailable("This feature is not available in SDK 10; use SDK 9 if you need it.")));

/**
 * Returns a string representation of the remote data dictionary. @see remoteData
 */
+ (NSString *) getRemoteDataJsonString __attribute__((unavailable("This feature is not available in SDK 10; use SDK 9 if you need it.")));


/**
 * Sets block which receives callbacks for all networks
 *
 */
+ (void) networkCallbackWithBlock: (void (^)(NSString *network, NSString *callback))block __attribute__((unavailable("This feature is not available in SDK 10; use SDK 9 if you need it.")));


/**
 *  Returns an `HZDemographics` object that you can use to pass demographic information to third party SDKs.
 *
 *  @return An `HZDemographics` object. Guaranteed to be non-nil after starting the SDK.
 */
+ (HZDemographics *)demographicInformation;

/**
 * Presents a view controller that displays integration information and allows fetch/show testing
 */
+ (void)presentMediationDebugViewController;


/**
 *  Retrieves the client for Fyber Virtual Currencies
 *
 *  @return The FYBVirtualCurrencyClient singleton
 *
 */
+ (FYBVirtualCurrencyClient *)virtualCurrencyClient;


#pragma mark - Performance Optimization

/**
 *  Call this method to have the SDK not start any expensive, main-thread operations. For example, when high-performance gameplay starts you might call `pauseExpensiveWork`, and then `resumeExpensiveWork` on the post-level screen.
 *
 *  Heyzap makes all possible efforts to move expensive work to background queues. We have profiled extensively with Timer Profiler and System Trace to try to minimize time spent on the main thread. If you run Instruments and see Heyzap spending more than 5ms on the main thread, please report this as a bug to Heyzap (and attach your .trace file if possible) (exceptions to this are while displaying ads and during the `start` call to Heyzap—we initialize the first `UIWebView` here to make subsequent ones cheaper; see http://stackoverflow.com/q/29811906/1176156).
 *
 *  However, certain operations are unavoidably expensive + must be performed on the main thread. For example, initializing some 3rd party SDKs can take up to 100ms. In some 3rd party networks, requesting an ad can take up to 60ms. Creating the first `UIWebView` in iOS takes up to 40ms, and subsequent ones take up to 11ms. This necessitates not starting these operations while 60 FPS gameplay is occurring.
 *
 *  If you are experiencing frame drops after adding mediation, you can use this method to prevent Heyzap from starting these expensive operations. Note that this could cause the time to finish a fetch take significantly longer. If you use this method, please take every opportunity to call `resumeExpensiveWork`; even spending a tenth of a second on a post-level screen is ample time for the most expensive operations to complete.
 *
 *  @warning Using this method is likely to extend the amount of time until you receive an ad from Heyzap Mediation. Please only use this method if you are experiencing performance issues and after reading this documentation.
 *  @note You *must* call `resumeExpensiveWork` to show ads after calling this.
 */
+ (void)pauseExpensiveWork;

/**
 *  Call this method to allow the SDK to start any expensive, main-thread operations. The SDK must be resumed before trying to show an ad.
 *
 *  @see pauseExpensiveWork
 */
+ (void)resumeExpensiveWork;


#pragma mark - Record IAP Transaction

/**
 * Call this method to record an In-App Purchase made from the user. This will disable Ads for the time interval set in your game settings.
 *
 * Only call this method if automatic IAP recording is disabled* (i.e. `HZAdOptionsDisableAutomaticIAPRecording` is enabled).
 */
+ (void)onIAPPurchaseComplete:(NSString *)productId productName:(NSString *)productName price:(NSDecimalNumber *)price;

@end
