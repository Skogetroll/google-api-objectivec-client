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
//  GTLService.h
//

// Service object documentation:
// https://code.google.com/p/google-api-objectivec-client/wiki/Introduction#Services_and_Tickets

#import <Foundation/Foundation.h>

#import "GTLDefines.h"

// Fetcher bridging macros -- Internal library use only.
//
// GTL_USE_SESSION_FETCHER should be set to force the GTL library to use
// GTMSessionFetcher rather than the older GTMHTTPFetcher.  The session
// fetcher requires iOS 7/OS X 10.9 and supports out-of-process uploads.

//#if (!TARGET_OS_IPHONE && defined(MAC_OS_X_VERSION_10_11) && MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_11) \
//|| (TARGET_OS_IPHONE && defined(__IPHONE_9_0) && __IPHONE_OS_VERSION_MAX_ALLOWED >= __IPHONE_9_0)
//#ifndef GTM_USE_SESSION_FETCHER
//#define GTM_USE_SESSION_FETCHER 1
//#endif
//#endif

#if !defined(GTL_USE_SESSION_FETCHER) && defined(GTM_USE_SESSION_FETCHER)
#define GTL_USE_SESSION_FETCHER GTM_USE_SESSION_FETCHER
#endif  // GTL_USE_SESSION_FETCHER

#if GTL_USE_SESSION_FETCHER
#define GTLUploadFetcherClass GTMSessionUploadFetcher
#define GTLUploadFetcherClassStr @"GTMSessionUploadFetcher"

#import "GTMSessionFetcher.h"
#import "GTMSessionFetcherService.h"
#else
// !GTL_USE_SESSION_FETCHER
#define GTLUploadFetcherClass GTMHTTPUploadFetcher
#define GTLUploadFetcherClassStr @"GTMHTTPUploadFetcher"

#import "GTMHTTPFetcher.h"
#import "GTMHTTPFetcherService.h"
#endif  // GTL_USE_SESSION_FETCHER

#import "GTLBatchQuery.h"
#import "GTLBatchResult.h"
#import "GTLDateTime.h"
#import "GTLErrorObject.h"
#import "GTLFramework.h"
#import "GTLJSONParser.h"
#import "GTLObject.h"
#import "GTLQuery.h"
#import "GTLUtilities.h"

// Error domains
extern NSString *__nonnull const kGTLServiceErrorDomain;
enum {
  kGTLErrorQueryResultMissing = -3000,
  kGTLErrorWaitTimedOut       = -3001
};

extern NSString *__nonnull const kGTLJSONRPCErrorDomain;

// We'll consistently store the server error string in the userInfo under
// this key
extern NSString *__nonnull const kGTLServerErrorStringKey;

extern Class __nonnull const kGTLUseRegisteredClass;

extern NSUInteger const kGTLStandardUploadChunkSize;

// When servers return us structured JSON errors, the NSError will
// contain a GTLErrorObject in the userInfo dictionary under the key
// kGTLStructuredErrorsKey
extern NSString *__nonnull const kGTLStructuredErrorKey;

// When specifying an ETag for updating or deleting a single entry, use
// kGTLETagWildcard to tell the server to replace the current value
// unconditionally.  Do not use this in entries in a batch feed.
extern NSString *__nonnull const kGTLETagWildcard;

// Notifications when parsing of a fetcher feed or entry begins or ends
extern NSString *__nonnull const kGTLServiceTicketParsingStartedNotification;
extern NSString *__nonnull const kGTLServiceTicketParsingStoppedNotification ;

@class GTLServiceTicket;

// Block types used for fetch callbacks

typedef void (^GTLServiceCompletionHandler)(GTLServiceTicket *__nonnull ticket, id __nullable object, NSError *__nullable error);

typedef void (^GTLServiceUploadProgressBlock)(GTLServiceTicket *__nonnull ticket,
                                              unsigned long long totalBytesUploaded,
                                              unsigned long long totalBytesExpectedToUpload);

typedef BOOL (^GTLServiceRetryBlock)(GTLServiceTicket *__nonnull ticket,
                                     BOOL suggestedWillRetry,
                                     NSError *__nullable error);

#pragma mark -

//
// Service base class
//

@interface GTLService : NSObject {
@private
  NSOperationQueue *parseQueue_;
  NSString *userAgent_;
  GTMBridgeFetcherService *fetcherService_;
  NSString *userAgentAddition_;
  
  NSMutableDictionary *serviceProperties_; // initial values for properties in future tickets
  
  NSDictionary *surrogates_; // initial value for surrogates in future tickets
  
  SEL uploadProgressSelector_; // optional
  
  GTLServiceRetryBlock retryBlock_;
  GTLServiceUploadProgressBlock uploadProgressBlock_;
  GTLQueryTestBlock testBlock_;
  
  NSUInteger uploadChunkSize_;      // zero when uploading via multi-part MIME http body
  
  BOOL isRetryEnabled_;             // user allows auto-retries
  SEL retrySelector_;               // optional; set with setServiceRetrySelector
  NSTimeInterval maxRetryInterval_; // default to 600. seconds
  
  BOOL shouldFetchNextPages_;
  
  BOOL allowInsecureQueries_;
  
  NSString *apiKey_;
  BOOL isRESTDataWrapperRequired_;
  NSString *apiVersion_;
  NSURL *rpcURL_;
  NSURL *rpcUploadURL_;
  NSDictionary *urlQueryParameters_;
  NSDictionary *additionalHTTPHeaders_;
  
#if GTL_USE_SESSION_FETCHER
  NSArray *runLoopModes_;
#endif
}

#pragma mark Query Execution

// The finishedSelector has a signature matching:
//
//   - (void)serviceTicket:(GTLServiceTicket *)ticket
//      finishedWithObject:(GTLObject *)object
//                   error:(NSError *)error
//
// If an error occurs, the error parameter will be non-nil.  Otherwise,
// the object parameter will point to a GTLObject, if any was returned by
// the fetch.  (Delete fetches return no object, so the second parameter will
// be nil.)
//
// If the query object is a GTLBatchQuery, the object passed to the callback
// will be a GTLBatchResult; see the batch query documentation:
// https://code.google.com/p/google-api-objectivec-client/wiki/Introduction#Batch_Operations

- (GTLServiceTicket *__nullable)executeQuery:(id<GTLQueryProtocol>__nonnull)query
                                    delegate:(id __nullable)delegate
                           didFinishSelector:(SEL __nullable)finishedSelector GTL_NONNULL((1));

- (GTLServiceTicket *__nullable)executeQuery:(id<GTLQueryProtocol> __nonnull)query
                           completionHandler:(GTLServiceCompletionHandler __nullable)handler GTL_NONNULL((1));

// Automatic page fetches
//
// Tickets can optionally do a sequence of fetches for queries where
// repeated requests with nextPageToken or nextStartIndex values is required to
// retrieve items of all pages of the response collection.  The client's
// callback is invoked only when all items have been retrieved, or an error has
// occurred.  During the fetch, the items accumulated so far are available from
// the ticket.
//
// Note that the final object may be a combination of multiple page responses
// so it may not be the same as if all results had been returned in a single
// page. Some fields of the response such as total item counts may reflect only
// the final page's values.
//
// Automatic page fetches will return an error if more than 25 page fetches are
// required.  For debug builds, this will log a warning to the console when more
// than 2 page fetches occur, as a reminder that the query's maxResults
// parameter should probably be increased to specify more items returned per
// page.
//
// Default value is NO.
@property (nonatomic, assign) BOOL shouldFetchNextPages;

// Retrying; see comments on retry support at the top of GTMHTTPFetcher.
//
// Default value is NO.
@property (nonatomic, assign, getter=isRetryEnabled) BOOL retryEnabled;

// Some services require a developer key for quotas and limits.  Setting this
// will include it on all request sent to this service via a GTLQuery class.
@property (nonatomic, copy) NSString *__nullable APIKey;

// An authorizer adds user authentication headers to the request as needed.
@property (nonatomic, retain) id <GTMFetcherAuthorizationProtocol> __nullable authorizer;

// Retry selector is optional for retries.
//
// If present, it should have the signature:
//   -(BOOL)ticket:(GTLServiceTicket *)ticket willRetry:(BOOL)suggestedWillRetry forError:(NSError *)error
// and return YES to cause a retry.  Note that unlike the fetcher retry
// selector, this selector's first argument is a ticket, not a fetcher.

@property (nonatomic, assign) SEL __nullable retrySelector;
@property (copy) GTLServiceRetryBlock __nullable retryBlock;

@property (nonatomic, assign) NSTimeInterval maxRetryInterval;

// A test block can be provided to test service calls without any network activity.
//
// See the description of GTLQueryTestBlock for additional details.
@property (nonatomic, copy) GTLQueryTestBlock __nullable testBlock;

//
// Fetches may be done using RPC or REST APIs, without creating
// a GTLQuery object
//

#pragma mark RPC Fetch Methods

//
// These methods may be used for RPC fetches without creating a GTLQuery object
//

- (GTLServiceTicket *__nullable)fetchObjectWithMethodNamed:(NSString *__nonnull)methodName
                                                parameters:(NSDictionary *__nullable)parameters
                                               objectClass:(Class __nullable)objectClass
                                                  delegate:(id __nullable)delegate
                                         didFinishSelector:(SEL __nullable)finishedSelector GTL_NONNULL((1));

- (GTLServiceTicket *__nullable)fetchObjectWithMethodNamed:(NSString *__nonnull)methodName
                                           insertingObject:(GTLObject *__nullable)bodyObject
                                               objectClass:(Class __nullable)objectClass
                                                  delegate:(id __nullable)delegate
                                         didFinishSelector:(SEL __nullable)finishedSelector GTL_NONNULL((1));

- (GTLServiceTicket *__nullable)fetchObjectWithMethodNamed:(NSString *__nonnull)methodName
                                                parameters:(NSDictionary *__nullable)parameters
                                           insertingObject:(GTLObject *__nullable)bodyObject
                                               objectClass:(Class __nullable)objectClass
                                                  delegate:(id __nullable)delegate
                                         didFinishSelector:(SEL __nullable)finishedSelector GTL_NONNULL((1));

- (GTLServiceTicket *__nullable)fetchObjectWithMethodNamed:(NSString *__nonnull)methodName
                                                parameters:(NSDictionary *__nullable)parameters
                                               objectClass:(Class __nullable)objectClass
                                         completionHandler:(GTLServiceCompletionHandler __nullable)handler GTL_NONNULL((1));

- (GTLServiceTicket *__nullable)fetchObjectWithMethodNamed:(NSString *__nonnull)methodName
                                           insertingObject:(GTLObject *__nullable)bodyObject
                                               objectClass:(Class __nullable)objectClass
                                         completionHandler:(GTLServiceCompletionHandler __nullable)handler GTL_NONNULL((1));

- (GTLServiceTicket *__nullable)fetchObjectWithMethodNamed:(NSString *__nonnull)methodName
                                                parameters:(NSDictionary *__nullable)parameters
                                           insertingObject:(GTLObject *__nullable)bodyObject
                                               objectClass:(Class __nullable)objectClass
                                         completionHandler:(GTLServiceCompletionHandler __nullable)handler GTL_NONNULL((1));

#pragma mark REST Fetch Methods

- (GTLServiceTicket *__nullable)fetchObjectWithURL:(NSURL *__nonnull)objectURL
                                          delegate:(id __nullable)delegate
                                 didFinishSelector:(SEL __nullable)finishedSelector GTL_NONNULL((1));

- (GTLServiceTicket *__nullable)fetchObjectWithURL:(NSURL *__nonnull)objectURL
                                       objectClass:(Class __nullable)objectClass
                                          delegate:(id __nullable)delegate
                                 didFinishSelector:(SEL __nullable)finishedSelector GTL_NONNULL((1));

- (GTLServiceTicket *__nullable)fetchPublicObjectWithURL:(NSURL *__nonnull)objectURL
                                             objectClass:(Class __nullable)objectClass
                                                delegate:(id __nullable)delegate
                                       didFinishSelector:(SEL __nullable)finishedSelector GTL_NONNULL((1));

- (GTLServiceTicket *__nullable)fetchObjectByInsertingObject:(GTLObject *__nonnull)bodyToPut
                                                      forURL:(NSURL *__nonnull)destinationURL
                                                    delegate:(id __nullable)delegate
                                           didFinishSelector:(SEL __nullable)finishedSelector GTL_NONNULL((1,2));

- (GTLServiceTicket *__nullable)fetchObjectByUpdatingObject:(GTLObject *__nonnull)bodyToPut
                                                     forURL:(NSURL *__nonnull)destinationURL
                                                   delegate:(id __nullable)delegate
                                          didFinishSelector:(SEL __nullable)finishedSelector GTL_NONNULL((1,2));

- (GTLServiceTicket *__nullable)deleteResourceURL:(NSURL *__nonnull)destinationURL
                                             ETag:(NSString *__nullable)etagOrNil
                                         delegate:(id __nullable)delegate
                                didFinishSelector:(SEL __nullable)finishedSelector GTL_NONNULL((1));

- (GTLServiceTicket *__nullable)fetchObjectWithURL:(NSURL *__nonnull)objectURL
                                 completionHandler:(GTLServiceCompletionHandler __nullable)handler GTL_NONNULL((1));

- (GTLServiceTicket *__nullable)fetchObjectByInsertingObject:(GTLObject *__nonnull)bodyToPut
                                                      forURL:(NSURL *__nonnull)destinationURL
                                           completionHandler:(GTLServiceCompletionHandler __nonnull)handler GTL_NONNULL((1));

- (GTLServiceTicket *__nullable)fetchObjectByUpdatingObject:(GTLObject *__nonnull)bodyToPut
                                                     forURL:(NSURL *__nonnull)destinationURL
                                          completionHandler:(GTLServiceCompletionHandler __nonnull)handler GTL_NONNULL((1));

- (GTLServiceTicket *__nullable)deleteResourceURL:(NSURL *__nonnull)destinationURL
                                             ETag:(NSString *__nullable)etagOrNil
                                completionHandler:(GTLServiceCompletionHandler __nullable)handler GTL_NONNULL((1));

#pragma mark User Properties

// Properties and userData are supported for client convenience.
//
// Property keys beginning with _ are reserved by the library.
//
// The service properties dictionary is copied to become the initial property
// dictionary for each ticket.
- (void)setServiceProperty:(id __nullable)obj forKey:(NSString *__nonnull)key GTL_NONNULL((2)); // pass nil obj to remove property
- (nullable id)servicePropertyForKey:(NSString *__nonnull)key GTL_NONNULL((1));

@property (nonatomic, copy) NSDictionary *__nullable serviceProperties;

// The service userData becomes the initial value for each future ticket's
// userData.
@property (nonatomic, retain) id __nullable serviceUserData;

#pragma mark Request Settings

// Set the surrogates to be used for future tickets.  Surrogates are subclasses
// to be used instead of standard classes when creating objects from the JSON.
// For example, this code will make the framework generate objects
// using MyCalendarItemSubclass instead of GTLItemCalendar and
// MyCalendarEventSubclass instead of GTLItemCalendarEvent.
//
//  NSDictionary *surrogates = @{
//    [GTLItemCalendar class] : [MyCalendarEntrySubclass class],
//    [GTLItemCalendarEvent class] : [MyCalendarEventSubclass class]
//  };
//  [calendarService setServiceSurrogates:surrogates];
//
@property (nonatomic, retain) NSDictionary *__nullable surrogates;

// On iOS 4 and later, the fetch may optionally continue in the background
// until finished or stopped by OS expiration.
//
// The default value is NO.
//
// For Mac OS X, background fetches are always supported, and this property
// is ignored.
@property (nonatomic, assign) BOOL shouldFetchInBackground;

// Callbacks can be invoked on an operation queue rather than via the run loop
// starting on 10.7 and iOS 6.  Do not specify both run loop modes and an
// operation queue. Specifying a delegate queue typically looks like this:
//
//   service.delegateQueue = [[[NSOperationQueue alloc] init] autorelease];
//
// Since the callbacks will be on a thread of the operation queue, the client
// may re-dispatch from the callbacks to a known dispatch queue or to the
// main queue.
@property (nonatomic, retain) NSOperationQueue *__nullable delegateQueue;

// Run loop modes are used for scheduling NSURLConnections.
//
// The default value, nil, schedules connections using the current run
// loop mode.  To use the service during a modal dialog, be sure to specify
// NSModalPanelRunLoopMode as one of the modes.
@property (nonatomic, retain) GTL_NSArrayOf(NSString *) *__nullable runLoopModes;

// Normally, API requests must be made only via SSL to protect the user's
// data and the authentication token.  This property allows the application
// to make non-SSL requests and localhost requests for testing.
//
// Defaults to NO.
@property (nonatomic, assign) BOOL allowInsecureQueries;

// Applications needing an additional identifier in the server logs may specify
// one.
@property (nonatomic, copy) NSString *__nullable userAgentAddition;

// Applications have a default user-agent based on the application signature
// in the Info.plist settings.  Most applications should not explicitly set
// this property.  Any string provided will be cleaned of inappropriate characters.
@property (nonatomic, copy) NSString *__nullable userAgent;

// The request user agent includes the library and OS version appended to the
// base userAgent, along with the optional addition string.
@property (nonatomic, readonly) NSString *__nullable requestUserAgent;

// Applications can provide a precise userAgent string identifying the application.
// No cleaning of characters is done.  Library-specific details will be appended.
- (void)setExactUserAgent:(NSString *__nonnull)userAgent;

// Applications may call requestForURL:httpMethod to get a request with the
// proper user-agent and ETag headers
//
// For http method, pass nil (for default GET method), POST, PUT, or DELETE
- (NSMutableURLRequest *__nullable)requestForURL:(NSURL *__nonnull)url
                                            ETag:(NSString *__nullable)etagOrNil
                                      httpMethod:(NSString *__nullable)httpMethodOrNil GTL_NONNULL((1));

// objectRequestForURL returns an NSMutableURLRequest for a JSON GTL object
//
// The object is the object being sent to the server, or nil;
// the http method may be nil for GET, or POST, PUT, DELETE
- (NSMutableURLRequest *__nonnull)objectRequestForURL:(NSURL *__nonnull)url
                                               object:(GTLObject *__nullable)object
                                                 ETag:(NSString *__nullable)etag
                                           httpMethod:(NSString *__nullable)httpMethod
                                               isREST:(BOOL)isREST
                                    additionalHeaders:(NSDictionary *__nullable)additionalHeaders
                                               ticket:(GTLServiceTicket *__nullable)ticket GTL_NONNULL((1));

// The queue used for parsing JSON responses (previously this property
// was called operationQueue)
@property (nonatomic, retain) NSOperationQueue *__nullable parseQueue;

// The fetcher service object issues the fetcher instances
// for this API service
@property (nonatomic, retain) GTMBridgeFetcherService *__nullable fetcherService;

// Default storage for cookies is in the service object's fetchHistory.
//
// Apps that want to share cookies between all standalone fetchers and the
// service object may specify static application-wide cookie storage,
// kGTMHTTPFetcherCookieStorageMethodStatic.
#if !GTL_USE_SESSION_FETCHER
@property (nonatomic, assign) NSInteger cookieStorageMethod;
#endif

// When sending REST style queries, should the payload be wrapped in a "data"
// element, and will the reply be wrapped in an "data" element.
@property (nonatomic, assign) BOOL isRESTDataWrapperRequired;

// Any url query parameters to add to urls (useful for debugging with some
// services).
@property (copy) GTL_NSDictionaryOf(NSString *, NSString *) *__nullable urlQueryParameters;

// Any extra http headers to set on requests for GTLObjects.
@property (copy) GTL_NSDictionaryOf(NSString *, NSString *) *__nullable additionalHTTPHeaders;

// The service API version.
@property (nonatomic, copy) NSString *__nullable apiVersion;

// The URL for sending RPC requests for this service.
@property (nonatomic, retain) NSURL *__nullable rpcURL;

// The URL for sending RPC requests which initiate file upload.
@property (nonatomic, retain) NSURL *__nullable rpcUploadURL;

// Set a non-zero value to enable uploading via chunked fetches
// (resumable uploads); typically this defaults to kGTLStandardUploadChunkSize
// for service subclasses that support chunked uploads
@property (nonatomic, assign) NSUInteger serviceUploadChunkSize;

// Service subclasses may specify their own default chunk size
+ (NSUInteger)defaultServiceUploadChunkSize;

// The service uploadProgressSelector becomes the initial value for each future
// ticket's uploadProgressSelector.
//
// The optional uploadProgressSelector will be called in the delegate as bytes
// are uploaded to the server.  It should have a signature matching
//
// - (void)ticket:(GTLServiceTicket *)ticket
//   hasDeliveredByteCount:(unsigned long long)numberOfBytesRead
//        ofTotalByteCount:(unsigned long long)dataLength;
@property (nonatomic, assign) SEL __nullable uploadProgressSelector;

@property (copy) GTLServiceUploadProgressBlock __nullable uploadProgressBlock;

@end

@interface GTLService (TestingSupport)

// Convenience method to create a mock GTL service just for testing.
//
// Queries executed by this mock service will not perform any network operation,
// but will invoke callbacks and provide the supplied data or error to the
// completion handler.
//
// You can make more customized mocks by setting the test block property of the service
// or query; the test block can inspect the query as ticket.originalQuery
//
// See the description of GTLQueryTestBlock for more details on customized testing.
//
// Example usage is in the unit test method testMockServiceConvenienceMethod.
+ (nonnull instancetype)mockServiceWithFakedObject:(id __nullable)objectOrNil
                                        fakedError:(NSError *__nullable)error;

// Wait synchronously for fetch to complete (strongly discouraged)
//
// This just runs the current event loop until the fetch completes
// or the timout limit is reached.  This may discard unexpected events
// that occur while spinning, so it's really not appropriate for use
// in serious applications.
//
// Returns true if an object was successfully fetched.  If the wait
// timed out, returns false and the returned error is nil.
//
// The returned object or error, if any, will be already autoreleased
//
// This routine will likely be removed in some future releases of the library.
- (BOOL)waitForTicket:(GTLServiceTicket *__nonnull)ticket
              timeout:(NSTimeInterval)timeoutInSeconds
        fetchedObject:(GTLObject *__nullable *__nullable)outObjectOrNil
                error:(NSError *__nullable *__nullable)outErrorOrNil GTL_NONNULL((1));

@end

#pragma mark -

//
// Ticket base class
//
@interface GTLServiceTicket : NSObject {
@private
  GTLService *service_;
  
  NSMutableDictionary *ticketProperties_;
  NSDictionary *surrogates_;
  
  GTMBridgeFetcher *objectFetcher_;
  SEL uploadProgressSelector_;
  BOOL shouldFetchNextPages_;
  BOOL isRetryEnabled_;
  SEL retrySelector_;
  NSTimeInterval maxRetryInterval_;
  
  GTLServiceRetryBlock retryBlock_;
  GTLServiceUploadProgressBlock uploadProgressBlock_;
  
  GTLObject *postedObject_;
  GTLObject *fetchedObject_;
  id<GTLQueryProtocol> executingQuery_;
  id<GTLQueryProtocol> originalQuery_;
  NSError *fetchError_;
  BOOL hasCalledCallback_;
  NSUInteger pagesFetchedCounter_;
  
  NSString *apiKey_;
  BOOL isREST_;
  
  NSOperation *parseOperation_;
}

+ (nonnull instancetype)ticketForService:(GTLService *__nonnull)service;

- (nonnull instancetype)initWithService:(GTLService *__nonnull)service NS_DESIGNATED_INITIALIZER;

@property (NS_NONATOMIC_IOSONLY, readonly, strong) id __nonnull service;

#pragma mark Execution Control
// if cancelTicket is called, the fetch is stopped if it is in progress,
// the callbacks will not be called, and the ticket will no longer be useful
// (though the client must still release the ticket if it retained the ticket)
- (void)cancelTicket;

// chunked upload tickets may be paused
- (void)pauseUpload;
- (void)resumeUpload;
@property (NS_NONATOMIC_IOSONLY, getter=isUploadPaused, readonly) BOOL uploadPaused;

@property (nonatomic, retain) GTMBridgeFetcher *__nullable objectFetcher;
@property (nonatomic, assign) SEL __nullable uploadProgressSelector;

// Services which do not require an user authorization may require a developer
// API key for quota management
@property (nonatomic, copy) NSString *__nullable APIKey;

#pragma mark User Properties

// Properties and userData are supported for client convenience.
//
// Property keys beginning with _ are reserved by the library.
- (void)setProperty:(id __nullable)obj forKey:(NSString *__nonnull)key GTL_NONNULL((2)); // pass nil obj to remove property
- (nullable id)propertyForKey:(NSString *__nonnull)key;

@property (nonatomic, copy) GTL_NSDictionaryOf(NSString *, id) *__nullable properties;
@property (nonatomic, retain) id __nullable userData;

#pragma mark Payload

@property (nonatomic, retain) GTLObject *__nullable postedObject;
@property (nonatomic, retain) GTLObject *__nullable fetchedObject;
@property (nonatomic, retain) id<GTLQueryProtocol> __nullable executingQuery; // Query currently being fetched by this ticket
@property (nonatomic, retain) id<GTLQueryProtocol> __nullable originalQuery;  // Query used to create this ticket
- (GTLQuery *__nullable)queryForRequestID:(NSString *__nonnull)requestID GTL_NONNULL((1)); // Returns the query from within the batch with the given id.

@property (nonatomic, retain) GTL_NSDictionaryOf(Class, Class) *__nullable surrogates;

#pragma mark Retry

@property (nonatomic, assign, getter=isRetryEnabled) BOOL retryEnabled;
@property (nonatomic, assign) SEL __nullable retrySelector;
@property (copy) GTLServiceRetryBlock __nullable retryBlock;
@property (nonatomic, assign) NSTimeInterval maxRetryInterval;

#pragma mark Status

@property (nonatomic, readonly) NSInteger statusCode; // server status from object fetch
@property (nonatomic, retain) NSError *__nullable fetchError;
@property (nonatomic, assign) BOOL hasCalledCallback;

#pragma mark Pagination

@property (nonatomic, assign) BOOL shouldFetchNextPages;
@property (nonatomic, assign) NSUInteger pagesFetchedCounter;

#pragma mark Upload

@property (copy) GTLServiceUploadProgressBlock __nullable uploadProgressBlock;

@end


// Category to provide opaque access to tickets stored in fetcher properties
@interface GTMBridgeFetcher (GTLServiceTicketAdditions)
- (nullable id)ticket;
@end

