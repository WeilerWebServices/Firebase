<!-- Do not edit this file. It is automatically generated by API Documenter. -->

[Home](./index.md) &gt; [@firebase/auth](./auth.md) &gt; [getMultiFactorResolver](./auth.getmultifactorresolver.md)

## getMultiFactorResolver() function

Provides a [MultiFactorResolver](./auth-types.multifactorresolver.md) suitable for completion of a multi-factor flow.

<b>Signature:</b>

```typescript
export declare function getMultiFactorResolver(auth: externs.Auth, error: externs.MultiFactorError): externs.MultiFactorResolver;
```

## Parameters

|  Parameter | Type | Description |
|  --- | --- | --- |
|  auth | externs.[Auth](./auth-types.auth.md) | The auth instance. |
|  error | externs.[MultiFactorError](./auth-types.multifactorerror.md) | The [MultiFactorError](./auth-types.multifactorerror.md) raised during a sign-in, or reauthentication operation. |

<b>Returns:</b>

externs.[MultiFactorResolver](./auth-types.multifactorresolver.md)
