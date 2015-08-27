/* Copyright (c) 2011 Google Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

//
// This sign-in object opens and closes the web view window as needed for
// users to sign in. For signing in to Google, it also obtains
// the authenticated user's email address.
//
// Typically, this will be managed for the application by
// GTMOAuth2ViewControllerTouch or GTMOAuth2WindowController, so this
// class's interface is interesting only if
// you are creating your own window controller for sign-in.
//
//
// Delegate methods implemented by the window controller
//
// The window controller implements two methods for use by the sign-in object,
// the webRequestSelector and the finishedSelector:
//
// webRequestSelector has a signature matching
//   - (void)signIn:(GTMOAuth2SignIn *)signIn displayRequest:(NSURLRequest *)request
//
// The web request selector will be invoked with a request to be displayed, or
// nil to close the window when the final callback request has been encountered.
//
//
// finishedSelector has a signature matching
//  - (void)signin:(GTMOAuth2SignIn *)signin finishedWithAuth:(GTMOAuth2Authentication *)auth error:(NSError *)error
//
// The finished selector will be invoked when sign-in has completed, except
// when explicitly canceled by calling cancelSigningIn
//

#if GTM_INCLUDE_OAUTH2 || !GDATA_REQUIRE_SERVICE_INCLUDES

#import <Foundation/Foundation.h>
#import <SystemConfiguration/SystemConfiguration.h>

#import "GTMOAuth2Authentication.h"

@interface GTMOAuth2SignIn : NSObject {
 @private
  GTMOAuth2Authentication *auth_;

  // the endpoint for displaying the sign-in page
  NSURL *authorizationURL_;
  NSDictionary *additionalAuthorizationParameters_;

  id delegate_;
  SEL webRequestSelector_;
  SEL finishedSelector_;

  BOOL hasHandledCallback_;

  GTMOAuth2Fetcher *pendingFetcher_;

#if !GTM_OAUTH2_SKIP_GOOGLE_SUPPORT
  BOOL shouldFetchGoogleUserEmail_;
  BOOL shouldFetchGoogleUserProfile_;
  NSDictionary *userProfile_;
#endif

  SCNetworkReachabilityRef reachabilityRef_;
  NSTimer *networkLossTimer_;
  NSTimeInterval networkLossTimeoutInterval_;
  BOOL hasNotifiedNetworkLoss_;

  id userData_;
}

@property (nonatomic, retain) GTMOAuth2Authentication *__nonnull authentication;

@property (nonatomic, retain) NSURL *__nonnull authorizationURL;
@property (nonatomic, retain) NSDictionary *__nonnull additionalAuthorizationParameters;

// The delegate is released when signing in finishes or is cancelled
@property (nonatomic, retain) id __nullable delegate;
@property (nonatomic, assign) SEL __nullable webRequestSelector;
@property (nonatomic, assign) SEL __nullable finishedSelector;

@property (nonatomic, retain) id __nullable userData;

// By default, signing in to Google will fetch the user's email, but will not
// fetch the user's profile.
//
// The email is saved in the auth object.
// The profile is available immediately after sign-in.
#if !GTM_OAUTH2_SKIP_GOOGLE_SUPPORT
@property (nonatomic, assign) BOOL shouldFetchGoogleUserEmail;
@property (nonatomic, assign) BOOL shouldFetchGoogleUserProfile;
@property (nonatomic, retain, readonly) NSDictionary *__nonnull userProfile;
#endif

// The default timeout for an unreachable network during display of the
// sign-in page is 30 seconds; set this to 0 to have no timeout
@property (nonatomic, assign) NSTimeInterval networkLossTimeoutInterval;

// The delegate is retained until sign-in has completed or been canceled
//
// designated initializer
- (nonnull instancetype)initWithAuthentication:(GTMOAuth2Authentication *__nonnull )auth
            authorizationURL:(NSURL *__nonnull)authorizationURL
                    delegate:(id __nullable)delegate
          webRequestSelector:(SEL __nullable)webRequestSelector
            finishedSelector:(SEL __nullable)finishedSelector NS_DESIGNATED_INITIALIZER;

// A default authentication object for signing in to Google services
#if !GTM_OAUTH2_SKIP_GOOGLE_SUPPORT
+ (GTMOAuth2Authentication *__nonnull)standardGoogleAuthenticationForScope:(NSString *__nonnull)scope
                                                         clientID:(NSString *__nonnull)clientID
                                                     clientSecret:(NSString *__nonnull)clientSecret;
#endif

#pragma mark Methods used by the Window Controller

// Start the sequence of fetches and sign-in window display for sign-in
@property (NS_NONATOMIC_IOSONLY, readonly) BOOL startSigningIn;

// Stop any pending fetches, and close the window (but don't call the
// delegate's finishedSelector)
- (void)cancelSigningIn;

// Window controllers must tell the sign-in object about any redirect
// requested by the web view, and any changes in the webview window title
//
// If these return YES then the event was handled by the
// sign-in object (typically by closing the window) and should be ignored by
// the window controller's web view

- (BOOL)requestRedirectedToRequest:(NSURLRequest *__nonnull)redirectedRequest;
- (BOOL)titleChanged:(NSString *__nonnull)title;
- (BOOL)cookiesChanged:(NSHTTPCookieStorage *__nonnull)cookieStorage;
- (BOOL)loadFailedWithError:(NSError *__nonnull)error;

// Window controllers must tell the sign-in object if the window was closed
// prematurely by the user (but not by the sign-in object); this calls the
// delegate's finishedSelector
- (void)windowWasClosed;

// Start the sequences for signing in with an authorization code. The
// authentication must contain an authorization code, otherwise the process
// will fail.
- (void)authCodeObtained;

#pragma mark -

#if !GTM_OAUTH2_SKIP_GOOGLE_SUPPORT
// Revocation of an authorized token from Google
+ (void)revokeTokenForGoogleAuthentication:(GTMOAuth2Authentication *__nonnull)auth;

// Create a fetcher for obtaining the user's Google email address or profile,
// according to the current auth scopes.
//
// The auth object must have been created with appropriate scopes.
//
// The fetcher's response data can be parsed with NSJSONSerialization.
+ (GTMOAuth2Fetcher *__nonnull)userInfoFetcherWithAuth:(GTMOAuth2Authentication *__nonnull)auth;

// Decode a web-safe Base64 encoded string.
+ (NSData *__nonnull)decodeWebSafeBase64:(NSString *__nonnull)base64Str;
#endif

#pragma mark -

// Standard authentication values
+ (NSString *__nonnull)nativeClientRedirectURI;
#if !GTM_OAUTH2_SKIP_GOOGLE_SUPPORT
+ (NSURL *__nonnull)googleAuthorizationURL;
+ (NSURL *__nonnull)googleTokenURL;
+ (NSURL *__nonnull)googleUserInfoURL;
#endif

@end

#endif // #if GTM_INCLUDE_OAUTH2 || !GDATA_REQUIRE_SERVICE_INCLUDES
