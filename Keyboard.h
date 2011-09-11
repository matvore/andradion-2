/*
Copyright 2011 Matt DeVore

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/

const int KEY_KEYCOUNT = 256;

void KeyInitialize(HINSTANCE hInstance, HWND hWnd);
void KeyRelease();

/** Gets the state of the keyboard so that the values returned by KeyPressed()
 * are according to the state as of the time this function is called.
 * If the module is not initialized, this function has no effect.
 */
void KeyRefreshState();

/** Indicates whether the given key is pressed down. If this function is called
 * when the module is not initialized, the return value is always false.
 * @param key an index in the range [0, KEY_KEYCOUNT) which indicates the keys
 *  whose index to get.
 * @return true iff the key was pressed the last time KeyRefreshState() was
 *  called.
 */
bool KeyPressed(int key);

/** Returns a user-readable name for the key index given. This method may be
 * called even when the Keyboard module is not initialized.
 * @param key an integer in the range [0, KEY_KEYCOUNT) indicating the index
 *  of the key whose name to retrieve.
 * @return a string containing the name of the given key.
 */
std::string KeyName(int key);
