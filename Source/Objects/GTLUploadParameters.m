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
//  GTLObject.m
//

#include <objc/runtime.h>

#import "GTLUploadParameters.h"

@implementation GTLUploadParameters

@synthesize MIMEType = MIMEType_,
            data = data_,
            fileHandle = fileHandle_,
            uploadLocationURL = uploadLocationURL_,
            fileURL = fileURL_,
            slug = slug_,
            shouldSendUploadOnly = shouldSendUploadOnly_,
            useBackgroundSession = useBackgroundSession_;

+ (instancetype)uploadParametersWithData:(NSData *)data
                                MIMEType:(NSString *)mimeType {
  GTLUploadParameters *params = [[[self alloc] init] autorelease];
  params.data = data;
  params.MIMEType = mimeType;
  params.useBackgroundSession = YES;
  return params;
}

+ (instancetype)uploadParametersWithFileHandle:(NSFileHandle *)fileHandle
                                      MIMEType:(NSString *)mimeType {
  GTLUploadParameters *params = [[[self alloc] init] autorelease];
  params.fileHandle = fileHandle;
  params.MIMEType = mimeType;
  params.useBackgroundSession = YES;
  return params;
}

+ (instancetype)uploadParametersWithFileURL:(NSURL *)fileURL
                                   MIMEType:(NSString *)mimeType {
  GTLUploadParameters *params = [[[self alloc] init] autorelease];
  params.fileURL = fileURL;
  params.MIMEType = mimeType;
  params.useBackgroundSession = YES;
  return params;
}

- (id)copyWithZone:(NSZone *)zone {
  GTLUploadParameters *newParams = [[[self class] allocWithZone:zone] init];
  newParams.MIMEType = self.MIMEType;
  newParams.data = self.data;
  newParams.fileHandle = self.fileHandle;
  newParams.fileURL = self.fileURL;
  newParams.uploadLocationURL = self.uploadLocationURL;
  newParams.slug = self.slug;
  newParams.shouldSendUploadOnly = self.shouldSendUploadOnly;
  newParams.useBackgroundSession = self.useBackgroundSession;
  return newParams;
}

- (void)dealloc {
  [MIMEType_ release];
  [data_ release];
  [fileHandle_ release];
  [fileURL_ release];
  [uploadLocationURL_ release];
  [slug_ release];

  [super dealloc];
}

#if DEBUG
- (NSString *)description {
  NSMutableArray *array = [NSMutableArray array];
  NSString *str = [NSString stringWithFormat:@"MIMEType:%@", MIMEType_];
  [array addObject:str];

  if (data_) {
    str = [NSString stringWithFormat:@"data:%llu bytes",
           (unsigned long long)data_.length];
    [array addObject:str];
  }

  if (fileHandle_) {
    str = [NSString stringWithFormat:@"fileHandle:%@", fileHandle_];
    [array addObject:str];
  }

  if (fileURL_) {
    str = [NSString stringWithFormat:@"file:%@", fileURL_.path];
    [array addObject:str];
  }

  if (uploadLocationURL_) {
    str = [NSString stringWithFormat:@"uploadLocation:%@",
           uploadLocationURL_.absoluteString];
    [array addObject:str];
  }

  if (slug_) {
    str = [NSString stringWithFormat:@"slug:%@", slug_];
    [array addObject:str];
  }

  if (shouldSendUploadOnly_) {
    [array addObject:@"shouldSendUploadOnly"];
  }

  if (useBackgroundSession_) {
    [array addObject:@"useBackgroundSession"];
  }

  NSString *descStr = [array componentsJoinedByString:@", "];
  str = [NSString stringWithFormat:@"%@ %p: {%@}",
         [self class], self, descStr];
  return str;
}
#endif

@end
