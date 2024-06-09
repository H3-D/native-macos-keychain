import * as os from 'os';

/** Native Lib API interface definition. */
interface INativeLibApi {
    
    /**
     * Sets the value of a keychain item. Adds item if does not exist.
     * 
     * @param key The keychain key. (kSecAttrAccount)
     * @param value The value. (kSecValueData)
     * @param service Optional: The keychain service/client name. (kSecAttrService)
     * @returns True if successful, otherwise false or throws an error for system related events.
     */
    setGenericPassword(key:string, value:string, service?:string) : boolean;

    /**
     * Returns the value of a keychain item.
     * 
     * @param key The keychain key. (kSecAttrAccount)
     * @param service Optional: The keychain service/client name. (kSecAttrService)
     * @returns The string value if found (kSecValueData), otherwise NULL r throws an error for system related events.
     */
    getGenericPassword(key:string, service?:string) : string;

    /**
     * Removes the value of a keychain item.
     * 
     * @param key The keychain key. (kSecAttrAccount)
     * @param service Optional: The keychain service/client name. (kSecAttrService)
     */
    removeGenericPassword(key:string, service?:string) : string;
}

/** Enumeration of update actions for keychain updates. */
export enum EUpdateKeychainAction {
    UPDATE = 'updated',
    REMOVE = 'removed'
}

/** Enumeration of keychain operation error types. */
export enum EKeychainOperationErrorType {
    SYSTEM    = 'system',
    NOT_FOUND = 'not_found'
}

/** Update keychain value params object definition. */
export interface IUpdateKeychainItemParams {
    /** Optional: The service/client name. (kSecAttrService) */
    service?:string;

    /** The key. (kSecAttrAccount) */
    key:string;

    /** The value. (kSecValueData) Pass as null or undefined to remove keychain item. */
    value?:string|null|undefined;
}

/** Get keychain value params object definition. */
export interface IGetKeychainItemParams {
    /** Optional: The service/client name. (kSecAttrService) */
    service?:string;

    /** The key. (kSecAttrAccount) */
    key:string;
}

/** Update keychain item result model definition. */
export interface IUpdateKeychainResult {
    /** The action taken or attempted. */
    actionPerformed:EUpdateKeychainAction;

    /** If the operation was successful. */
    success:boolean;

    /** Error type. */
    errorType?: EKeychainOperationErrorType;

    /** Thrown error if native operation failed due to a system error. */
    error?:Error;
}

/** Get keychain item result model definition. */
export interface IGetKeychainResult {
    /** The string value, if found. */
    value?:string;

    /** If the operation was successful. */
    success:boolean;

    /** Error type. */
    errorType?: EKeychainOperationErrorType;

    /** Thrown error if native operation failed due to a system error. */
    error?:Error;
}

/** Main interface class for binary  */
export class NativeKeychain {

    /** Static reference to native lib. */
    private static mExtRef:INativeLibApi;

    /**
     * Utility method for initializing native lib reference based on current platform.
     * 
     * @throws Error if current platform is not supported.
     */
    private static initialize() : void {
        
        // Verify extension reference has not been set.
        if(NativeKeychain.mExtRef) { return; }

        // Verify platform.
        if(os.platform() !== 'darwin') {
            throw(new Error('Invalid platform. OS must be MacOS. (darwin)'));
        }

        // Set linked reference for native lib based on Arch.
        switch(process.arch) {
            case 'arm64' : {
                NativeKeychain.mExtRef = require(`${__dirname}/../lib/arm64/native-macos-keychain`);
            }
            break;

            case 'x64' : {
                NativeKeychain.mExtRef = require(`${__dirname}/../x86_64/native-macos-keychain`);
            }
            break;

            default : {
                throw Error(`Invalid platform. Unsupported architecture (${process.arch})`);
            }
        }
    }

    /**
     * Sets (or) removes a keychain generic password keychain value.
     * - If keychain value is not current set, will create the keychain item.
     * - If params.value is passed as null (or) undefined, the value, if exists, will be removed.
     * 
     * @param params Update params model instance
     * @returns IUpdateKeychainResult result model.
     * @throws Error if current platform not supported
     */
    public setGenericPassword(params:IUpdateKeychainItemParams) : IUpdateKeychainResult {

        // Verify external reference initialized
        if(!NativeKeychain.mExtRef) {
            NativeKeychain.initialize();
        } 

        // Update if value is not null or undefined
        if(params.value !== null && params.value !== undefined) {
            try {
                // Call set password on native reference.
                const result = (params.service) ?
                    NativeKeychain.mExtRef.setGenericPassword(params.key, params.value, params.service)
                    :
                    NativeKeychain.mExtRef.setGenericPassword(params.key, params.value)
                ;

                // Check for failure
                if(!result) {
                    return {
                        actionPerformed : EUpdateKeychainAction.UPDATE,
                        success         : false,
                        errorType       : EKeychainOperationErrorType.NOT_FOUND
                    };
                }

                // Return success
                return {
                    actionPerformed : EUpdateKeychainAction.UPDATE,
                    success         : true
                };
            }
            catch(err:any) {
                // Return system error
                return {
                    actionPerformed : EUpdateKeychainAction.UPDATE,
                    success         : false,
                    errorType       : EKeychainOperationErrorType.SYSTEM,
                    error           : err
                };
            }
        }

        // Otherwise, remove item from keychain
        try {
            const result = (params.service) ?
                NativeKeychain.mExtRef.removeGenericPassword(params.key, params.service)
                :
                NativeKeychain.mExtRef.removeGenericPassword(params.key)
            ;

            // Check for failure
            if(!result) {
                return {
                    actionPerformed : EUpdateKeychainAction.REMOVE,
                    success         : false,
                    errorType       : EKeychainOperationErrorType.NOT_FOUND
                };
            }

            // Return success
            return {
                actionPerformed : EUpdateKeychainAction.REMOVE,
                success         : true
            };
        }
        catch(err:any) {
            // Return system error
            return {
                actionPerformed : EUpdateKeychainAction.REMOVE,
                success         : false,
                errorType       : EKeychainOperationErrorType.SYSTEM,
                error           : err
            };
        }
    }


    /**
     * Returns the value for a generic password keychain item.
     * 
     * @param params Get params model instance
     * @returns IGetKeychainResult result model.
     * @throws Error if current platform not supported
     */
    public getGenericPassword(params:IGetKeychainItemParams) : IGetKeychainResult {

        // Verify external reference initialized
        if(!NativeKeychain.mExtRef) {
            NativeKeychain.initialize();
        }

        
        try {
            // Pull value
            const result = (params.service) ?
                NativeKeychain.mExtRef.getGenericPassword(params.key, params.service)
                :
                NativeKeychain.mExtRef.getGenericPassword(params.key)
            ;

            // Check for failure
            if(result === null || result === undefined) {
                return {
                    success   : false,
                    errorType : EKeychainOperationErrorType.NOT_FOUND
                };
            }

            // Return success
            return {
                value   : result,
                success : true
            };

        }
        catch(err:any) {
            // Return system error
            return {
                success         : false,
                errorType       : EKeychainOperationErrorType.SYSTEM,
                error           : err
            };
        }
    }
}