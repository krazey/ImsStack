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

package com.android.imsstack.enabler.ssc;

import org.w3c.dom.Node;
import org.w3c.dom.NodeList;

import java.util.ArrayList;
import java.util.List;

/**
 * A custom, modifiable implementation of the {@code org.w3c.dom.NodeList} interface, implemented
 * using an {@link ArrayList}.
 */
public class SscNodeListImpl  implements NodeList {
    private final List<Node> mList;

    public SscNodeListImpl() {
        mList = new ArrayList<Node>();
    }

    /**
     * Appends a {@code Node} to the end of this node list.
     * <p>
     * This is a custom convenience method for populating the {@code SscNodeListImpl} instance,
     * as the standard {@code org.w3c.dom.NodeList} interface is read-only and does not define
     * methods for adding items.
     *
     * @param node The {@code Node} to add to the end of the list
     */
    public void add(Node node) {
        mList.add(node);
    }

    @Override
    public Node item(int index) {
        if (index < 0 || index >= mList.size()) {
            return null;
        }

        return mList.get(index);
    }

    @Override
    public int getLength() {
        return mList.size();
    }
}
