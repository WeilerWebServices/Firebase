## API Report File for "@firebase/remote-config-exp"

> Do not edit this file. It is a report generated by [API Extractor](https://api-extractor.com/).

```ts

import { FirebaseApp } from '@firebase/app-types-exp';
import { LogLevel } from '@firebase/remote-config-types-exp';
import { RemoteConfig } from '@firebase/remote-config-types-exp';
import { Value } from '@firebase/remote-config-types-exp';

// @public
export function activate(remoteConfig: RemoteConfig): Promise<boolean>;

// @public
export function ensureInitialized(remoteConfig: RemoteConfig): Promise<void>;

// @public
export function fetchAndActivate(remoteConfig: RemoteConfig): Promise<boolean>;

// @public
export function fetchConfig(remoteConfig: RemoteConfig): Promise<void>;

// @public
export function getAll(remoteConfig: RemoteConfig): Record<string, Value>;

// @public
export function getBoolean(remoteConfig: RemoteConfig, key: string): boolean;

// @public
export function getNumber(remoteConfig: RemoteConfig, key: string): number;

// @public (undocumented)
export function getRemoteConfig(app: FirebaseApp): RemoteConfig;

// @public
export function getString(remoteConfig: RemoteConfig, key: string): string;

// @public
export function getValue(remoteConfig: RemoteConfig, key: string): Value;

// @public
export function setLogLevel(remoteConfig: RemoteConfig, logLevel: LogLevel): void;


// (No @packageDocumentation comment for this package)

```