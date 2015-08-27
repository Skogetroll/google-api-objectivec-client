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
//  GTLObject.h
//

// GTLObject documentation:
// https://code.google.com/p/google-api-objectivec-client/wiki/Introduction#Objects_and_Queries

#import <Foundation/Foundation.h>

#import "GTLDefines.h"
#import "GTLUtilities.h"
#import "GTLDateTime.h"

@class GTLObject;

@protocol GTLCollectionProtocol
@optional
@property (retain) GTL_NSArrayOf(GTLObject *) *__nonnull items;
@end

@protocol GTLBatchItemCreationProtocol
- (void)createItemsWithClassMap:(NSDictionary *__nonnull)batchClassMap;
@end

@interface GTLObject : NSObject <NSCopying, NSSecureCoding> {
@private
  NSMutableDictionary *json_;
  
  // Used when creating the subobjects from this one.
  NSDictionary *surrogates_;
  
  // Any complex object hung off this object goes into the cache so the
  // next fetch will get the same object back instead of having to recreate
  // it.
  NSMutableDictionary *childCache_;
  
  // Anything defined by the client; retained but not used internally; not
  // copied by copyWithZone:
  NSMutableDictionary *userProperties_;
}

@property (nonatomic, retain) GTL_NSMutableDictionaryOf(id, id) *__nullable JSON;
@property (nonatomic, retain) GTL_NSDictionaryOf(Class, Class) *__nullable surrogates;
@property (nonatomic, retain) GTL_NSMutableDictionaryOf(NSString *, id) *__nullable userProperties;

///////////////////////////////////////////////////////////////////////////////
//
// Public methods
//
// These methods are intended for users of the library
//

+ (nonnull instancetype)object;
+ (nonnull instancetype)objectWithJSON:(NSMutableDictionary *__nonnull)dict;

- (nullable id)copyWithZone:(NSZone *__nullable)zone;

@property (NS_NONATOMIC_IOSONLY, readonly, copy) NSString *__nullable JSONString;

// generic access to json; also creates it if necessary
- (void)setJSONValue:(id __nullable)obj forKey:(NSString *__nonnull)key  GTL_NONNULL((2));
- (__nullable id)JSONValueForKey:(NSString *__nonnull)key;

// Returns the list of keys in this object's JSON that aren't listed as
// properties on the object.
@property (NS_NONATOMIC_IOSONLY, readonly, copy) NSArray *__nullable additionalJSONKeys;

// Any keys in the JSON that aren't listed as @properties on the object
// are counted as "additional properties".  These allow you to get/set them.
- (nullable id)additionalPropertyForName:(NSString *__nonnull)name;
- (void)setAdditionalProperty:(id __nullable)obj forName:(NSString *__nonnull)name GTL_NONNULL((2));
@property (NS_NONATOMIC_IOSONLY, readonly, copy) NSDictionary *__nullable additionalProperties;

// User properties are supported for client convenience, but are not copied by
// copyWithZone.  User Properties keys beginning with _ are reserved by the library.
//
// Set nil for obj to remove the property.
- (void)setProperty:(id __nullable)obj forKey:(NSString *__nonnull)key GTL_NONNULL((2));
- (nullable id)propertyForKey:(NSString *__nonnull)key GTL_NONNULL((1));

// userData is stored as a property with key "_userData"
@property (NS_NONATOMIC_IOSONLY, strong) id __nullable userData;

// Makes a partial query-compatible string describing the fields present
// in this object. (Note: only the first element of any array is examined.)
//
// http://code.google.com/apis/tasks/v1/performance.html#partial
//
@property (NS_NONATOMIC_IOSONLY, readonly, copy) NSString *__nullable fieldsDescription;

// Makes an object containing only the changes needed to do a partial update
// (patch), where the patch would be to change an object from the original
// to the receiver, such as
//
// GTLSomeObject *patchObject = [newVersion patchObjectFromOriginal:oldVersion];
//
// http://code.google.com/apis/tasks/v1/performance.html#patch
//
// NOTE: this method returns nil if there are no changes between the original
// and the receiver.
- (nullable id)patchObjectFromOriginal:(GTLObject *__nonnull)original;

// Method creating a null value to set object properties for patch queries that
// delete fields.  Do not use this except when setting an object property for
// a patch query.
+ (nonnull id)nullValue;

///////////////////////////////////////////////////////////////////////////////
//
// Protected methods
//
// These methods are intended for subclasses of GTLObject
//

// class registration ("kind" strings) for subclasses
+ (nullable Class)registeredObjectClassForKind:(NSString *__nonnull)kind;
+ (void)registerObjectClassForKind:(NSString *__nonnull)kind;

// creation of objects from a JSON dictionary
+ (GTLObject *__nonnull)objectForJSON:(NSMutableDictionary *__nonnull)json
                         defaultClass:(Class __nonnull)defaultClass
                           surrogates:(NSDictionary *__nullable)surrogates
                        batchClassMap:(NSDictionary *__nullable)batchClassMap;

// property-to-key mapping (for JSON keys which are not used as method names)
+ (NSDictionary *__nullable)propertyToJSONKeyMap;

// property-to-Class mapping for array properties (to say what is in the array)
+ (NSDictionary *__nullable)arrayPropertyToClassMap;

// The default class for additional JSON keys
+ (nullable Class)classForAdditionalProperties;

@end

// Collection objects with an "items" property should derive from GTLCollection
// object.  This provides support for fast object enumeration, the
// itemAtIndex: convenience method, and indexed subscripts.
//
// Subclasses must implement the items method dynamically.
@interface GTLCollectionObject : GTLObject <GTLCollectionProtocol, NSFastEnumeration> {
@private
  NSDictionary *identifierMap_;
}

// itemAtIndex: and objectAtIndexedSubscript: return nil when the index exceeds
// the bounds of the items array.
- (nullable id)itemAtIndex:(NSUInteger)idx;

- (nullable id)objectAtIndexedSubscript:(NSInteger)idx;

// itemForIdentifier: looks up items from the collection object by identifier,
// and returns the first one.
//
// Typically, items will have a unique identifier (with key "id" in the
// object's JSON).  This method returns the first item found in the collection
// with the specified identifier.
//
// The first time this method is used, the collection will cache a map of
// identifiers to items.  If the items list for the instance somehow changes,
// use the reset method below to force a new cache to be created for this
// collection.
- (nullable id)itemForIdentifier:(NSString *__nonnull)key GTL_NONNULL((1));

// Identifiers for all items are cached when the first one is obtained.
// This method resets the cache.  It is needed only if the item list has
// changed.
- (void)resetIdentifierMap;

@end

@interface GTLCollectionObject (DynamicMethods)
@property (NS_NONATOMIC_IOSONLY, readonly, copy) NSArray *__nonnull items;
@end

// Base object use for when an service method directly returns an array instead
// of an object. Normally methods should return an object with an 'items'
// property, but this exists for the methods not up to spec.
@interface GTLResultArray : GTLCollectionObject
// This method should only be called by subclasses.
- (NSArray *__nullable)itemsWithItemClass:(Class __nonnull)itemClass;
@end
