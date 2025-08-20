/*
 * Copyright (C) 2025 The Android Open Source Project
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

package com.android.imsstack.enabler.mtc;

import android.os.Parcel;

import java.util.LinkedHashMap;
import java.util.List;

/**
 * This class provides utility methods to manage {@link SuppService} items.
 * <p>
 * It includes functionality for adding, removing, updating, and querying
 * supplementary services stored in a list.
 */
public final class SuppServiceUtils {
    public static final int TYPE_BOOLEAN = 1;
    public static final int TYPE_INT = 2;
    public static final int TYPE_STRING = 3;

    private static final LinkedHashMap<Integer, String> sTypeToKey =
            new LinkedHashMap<Integer, String>();
    private static final LinkedHashMap<Integer, Integer> sTypeToValueType =
            new LinkedHashMap<Integer, Integer>();

    /**
     * Adds a mapping from a supplementary service type to a key string.
     * This is useful for associating a service type integer with a descriptive name.
     *
     * @param type The integer representing the supplementary service type.
     * @param key The key string to associate with the service type.
     */
    public static void addKey(int type, String key) {
        sTypeToKey.put(type, key);
    }

    /**
     * Removes the key string associated with a given supplementary service type.
     *
     * @param type The integer representing the supplementary service type.
     */
    public static void removeKey(int type) {
        sTypeToKey.remove(type);
    }

    /**
     * Adds a mapping from a supplementary service type to its value type (boolean, int, or string).
     *
     * @param type The integer representing the supplementary service type.
     * @param valueType The type of value associated with the service (e.g., TYPE_BOOLEAN).
     */
    public static void addValueType(int type, int valueType) {
        sTypeToValueType.put(type, valueType);
    }

    /**
     * Removes the value type mapping for a given supplementary service type.
     *
     * @param type The integer representing the supplementary service type.
     */
    public static void removeValueType(int type) {
        sTypeToValueType.remove(type);
    }

    /**
     * Retrieves the key string for a given supplementary service type.
     *
     * @param type The integer representing the supplementary service type.
     * @return The key string associated with the type, or null if not found.
     */
    public static String getKey(int type) {
        return sTypeToKey.get(type);
    }

    /**
     * Checks if the value type for a given supplementary service type is a boolean.
     *
     * @param type The integer representing the supplementary service type.
     * @return {@code true} if the value type is a boolean, otherwise {@code false}.
     */
    public static boolean isValueBoolean(int type) {
        return sTypeToValueType.getOrDefault(type, -1) == TYPE_BOOLEAN;
    }

    /**
     * Checks if the value type for a given supplementary service type is an integer.
     *
     * @param type The integer representing the supplementary service type.
     * @return {@code true} if the value type is an integer, otherwise {@code false}.
     */
    public static boolean isValueInt(int type) {
        return sTypeToValueType.getOrDefault(type, -1) == TYPE_INT;
    }

    /**
     * Checks if the value type for a given supplementary service type is a string.
     *
     * @param type The integer representing the supplementary service type.
     * @return {@code true} if the value type is a string, otherwise {@code false}.
     */
    public static boolean isValueString(int type) {
        return sTypeToValueType.getOrDefault(type, -1) == TYPE_STRING;
    }

    /**
     * Represents a single supplementary service with a type and three value fields.
     */
    public static class SuppService {
        /** The type of the supplementary service. */
        public int type;
        /** The string value for the service, if applicable. */
        public String strValue;
        /** The integer value for the service, if applicable. */
        public int intValue;
        /** The boolean value for the service, if applicable. */
        public boolean boolValue;
    }

    /**
     * Adds a new {@link SuppService} item with all value fields to a list.
     *
     * @param suppServices The list to which the service will be added.
     * @param type The type of the service.
     * @param boolValue The boolean value of the service.
     * @param intValue The integer value of the service.
     * @param strValue The string value of the service.
     */
    public static void addService(List<SuppService> suppServices,
            int type, boolean boolValue, int intValue, String strValue) {
        SuppService service = new SuppService();
        service.type = type;
        service.strValue = strValue;
        service.intValue = intValue;
        service.boolValue = boolValue;

        suppServices.add(service);
    }

    /**
     * Adds all supplementary services from a source list to a destination list.
     * Each service is cloned before being added to the destination list.
     *
     * @param sourceSupps The list of services to be copied.
     * @param destinationSupps The list to which the services will be added.
     */
    public static void addServices(List<SuppService> sourceSupps,
            List<SuppService> destinationSupps) {
        for (SuppService sourceService : sourceSupps) {
            addService(destinationSupps, sourceService.type,
                    sourceService.boolValue, sourceService.intValue,
                    sourceService.strValue);
        }
    }

    /**
     * Adds or updates a string-valued supplementary service in a list.
     * If a service with the same type already exists, its string value is updated.
     * Otherwise, a new service is added.
     *
     * @param suppServices The list of services to modify.
     * @param type The type of the service to add or update.
     * @param value The new string value.
     */
    public static void addServiceStr(List<SuppService> suppServices, int type, String value) {
        if (isService(suppServices, type)) {
            SuppService service = getService(suppServices, type);
            service.strValue = value;
        } else {
            SuppService newService = new SuppService();
            newService.type = type;
            newService.strValue = value;
            suppServices.add(newService);
        }
    }

    /**
     * Adds or updates an integer-valued supplementary service in a list.
     * If a service with the same type already exists, its integer value is updated.
     * Otherwise, a new service is added.
     *
     * @param suppServices The list of services to modify.
     * @param type The type of the service to add or update.
     * @param value The new integer value.
     */
    public static void addServiceInt(List<SuppService> suppServices, int type, int value) {
        if (isService(suppServices, type)) {
            SuppService service = getService(suppServices, type);
            service.intValue = value;
        } else {
            SuppService newService = new SuppService();
            newService.type = type;
            newService.intValue = value;
            suppServices.add(newService);
        }
    }

    /**
     * Adds or updates a boolean-valued supplementary service in a list.
     * If a service with the same type already exists, its boolean value is updated.
     * Otherwise, a new service is added.
     *
     * @param suppServices The list of services to modify.
     * @param type The type of the service to add or update.
     * @param value The new boolean value.
     */
    public static void addServiceBool(List<SuppService> suppServices, int type, boolean value) {
        if (isService(suppServices, type)) {
            SuppService service = getService(suppServices, type);
            service.boolValue = value;
        } else {
            SuppService newService = new SuppService();
            newService.type = type;
            newService.boolValue = value;
            suppServices.add(newService);
        }
    }

    /**
     * Updates an existing service in a list, or adds it if it does not exist.
     * The update is based on matching the service type.
     *
     * @param suppServices The list of services to modify.
     * @param sourceService The service containing the new values.
     */
    public static void updateService(List<SuppService> suppServices, SuppService sourceService) {
        boolean bUpdated = false;

        for (SuppService targetService : suppServices) {
            if (targetService.type == sourceService.type) {
                targetService.strValue = sourceService.strValue;
                targetService.intValue = sourceService.intValue;
                targetService.boolValue = sourceService.boolValue;
                bUpdated = true;
                break;
            }
        }

        if (!bUpdated) {
            SuppService service = new SuppService();
            service.type = sourceService.type;
            service.strValue = sourceService.strValue;
            service.intValue = sourceService.intValue;
            service.boolValue = sourceService.boolValue;

            suppServices.add(service);
        }
    }

    /**
     * Updates a destination list of services by copying all services from a source list.
     * This method clears the destination list first, then adds or updates services from the source.
     *
     * @param sourceSupps The source list containing services to be copied.
     * @param destinationSupps The destination list to be updated.
     */
    public static void updateServices(List<SuppService> sourceSupps,
            List<SuppService> destinationSupps) {
        destinationSupps.clear();
        for (SuppService sourceService : sourceSupps) {
            updateService(destinationSupps, sourceService);
        }
    }

    /**
     * Checks if a service with the specified type exists in a list.
     *
     * @param suppServices The list of services to search.
     * @param type The service type to look for.
     * @return {@code true} if a service with the given type is found, otherwise {@code false}.
     */
    public static boolean isService(List<SuppService> suppServices, int type) {
        return getService(suppServices, type) != null;
    }

    /**
     * Retrieves a service with the specified type from a list.
     *
     * @param suppServices The list of services to search.
     * @param type The service type to look for.
     * @return The {@link SuppService} object if found, otherwise {@code null}.
     */
    public static SuppService getService(List<SuppService> suppServices, int type) {
        for (SuppService service : suppServices) {
            if (service.type == type) {
                return service;
            }
        }

        return null;
    }

    /**
     * Reads a list of supplementary services from a Parcel.
     * The existing content of the list is cleared before reading.
     *
     * @param suppServices The list to populate with services from the Parcel.
     * @param source The Parcel to read from.
     */
    public static void readSuppFromParcel(List<SuppService> suppServices, Parcel source) {
        int serviceCount = source.readInt();
        suppServices.clear();

        for (int index = 0; index < serviceCount; index++) {
            SuppServiceUtils.SuppService service = new SuppServiceUtils.SuppService();

            service.type = source.readInt();
            service.strValue = source.readString();
            service.intValue = source.readInt();
            service.boolValue = (source.readInt() == 1);

            suppServices.add(service);
        }
    }

    /**
     * Writes a list of supplementary services to a Parcel.
     *
     * @param suppServices The list of services to write.
     * @param dest The Parcel to write to.
     */
    public static void writeSuppToParcel(List<SuppService> suppServices, Parcel dest) {
        dest.writeInt(suppServices.size());

        for (SuppService service : suppServices) {
            dest.writeInt(service.type);
            dest.writeString(service.strValue);
            dest.writeInt(service.intValue);
            dest.writeInt(service.boolValue ? 1 : 0);
        }
    }
}
