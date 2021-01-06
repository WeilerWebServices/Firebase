/*
 * Copyright 2020 Google Inc. All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
import * as firebase from '@firebase/rules-unit-testing';
import * as path from 'path';
import * as fs from 'fs';

export function readRulesFile(name: string): string {
  return fs.readFileSync(getRulesFilePath(name), 'utf8');
}

export function getRulesFilePath(name: string): string {
  return path.join(__dirname, name);
}

export function getAuthedDb(project: string, uid: string | any) {
  const auth = uid ? { uid } : undefined;
  return firebase
    .initializeTestApp({
      projectId: project,
      auth: auth
    })
    .firestore();
}
