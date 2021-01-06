/**
 * @license
 * Copyright 2017 Google LLC
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
import { TaskState } from './implementation/taskenums';
import { Metadata } from './metadata';
import { StorageReference } from './reference';
import { UploadTask } from './task';

/**
 * Result returned from a non-resumable upload.
 * @public
 */
export interface UploadResult {
  readonly metadata: Metadata;
  readonly ref: StorageReference;
}

/**
 * Holds data about the current state of the upload task.
 * @public
 */
export interface UploadTaskSnapshot {
  readonly bytesTransferred: number;
  readonly totalBytes: number;
  readonly state: TaskState;
  readonly metadata: Metadata;
  readonly task: UploadTask;
  readonly ref: StorageReference;
}