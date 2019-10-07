#if defined _easyjson_included
 #endinput
#endif
#define _easyjson_included

enum JSONType
{
	Type_String,
	Type_Integer,
	Type_Float,
	Type_Object,
	Type_Boolean,
	Type_Array
}

stock Handle CreateJSON()
{
	Handle m_hJSON = CreateTrie();
	Handle m_hJSONKeys = CreateArray(512);
	SetTrieValue(m_hJSON, "json_key_array", m_hJSONKeys);
	return m_hJSON;
}

stock Handle DecodeJSON(const char[] json, int len = 0)
{
	int m_iPos, m_iIgnore;
	int m_iLength = strlen(json);

	m_iPos = JSONIgnore(json);
	if (m_iPos == -1 || json[m_iPos] != '{')
		return INVALID_HANDLE;
	m_iPos++;
	m_iIgnore = JSONIgnore(json[m_iPos]);
	if (m_iIgnore == -1)
		return INVALID_HANDLE;
	m_iPos += m_iIgnore;
	if (json[m_iPos] == '}')
		return INVALID_HANDLE;

	int m_iStringStart;
	int m_iStringEnd;

	bool m_bEnd = false;

	Handle m_hJSON = CreateJSON();
	while (m_iPos<m_iLength && json[m_iPos] != '}' && !m_bEnd)
	{
		if (GetJSONString(json[m_iPos], m_iStringStart, m_iStringEnd))
		{
			char[] m_szKey = new char[m_iStringEnd - m_iStringStart + 1];
			strcopy(m_szKey, m_iStringEnd-m_iStringStart+1, json[m_iPos+m_iStringStart]);
			m_iPos += m_iStringEnd + 1;
			m_iPos += JSONIgnore(json[m_iPos]);

			if (json[m_iPos] == '{')
			{
				int m_iLen = -1;
				Handle m_hObject = DecodeJSON(json[m_iPos], m_iLen);
				if (m_hObject != INVALID_HANDLE)
				{
					JSONSetObject(m_hJSON, m_szKey, m_hObject);
					m_iPos += m_iLen;
					m_iPos += JSONIgnore(json[m_iPos]);
				}
				else
				{
					DestroyJSON(m_hJSON);
					return INVALID_HANDLE;
				}
			}
			if (json[m_iPos] == '[')
			{
				int m_iLen = -1;
				Handle m_hObject = DecodeArray(json[m_iPos], m_iLen);
				if (m_hObject != INVALID_HANDLE)
				{
					JSONSetArray(m_hJSON, m_szKey, m_hObject);
					m_iPos += m_iLen;
					m_iPos += JSONIgnore(json[m_iPos]);
				}
				else
				{
					DestroyJSON(m_hJSON);
					return INVALID_HANDLE;
				}
			}
			else if (GetJSONString(json[m_iPos], m_iStringStart, m_iStringEnd))
			{
				char[] m_szValue = new char[m_iStringEnd - m_iStringStart + 1];
				strcopy(m_szValue, m_iStringEnd - m_iStringStart+  1, json[m_iPos + m_iStringStart]);
				
				if (json[m_iPos + m_iStringEnd] == '}')
					m_bEnd = true;
				m_iPos += m_iStringEnd + 1;
				m_iPos += JSONIgnore(json[m_iPos]);

				int m_iDot = 0;

				if (m_iStringStart != 0)
					JSONSetString(m_hJSON, m_szKey, m_szValue);
				else if (strcmp(m_szValue, "true") == 0)
					JSONSetBoolean(m_hJSON, m_szKey, true);
				else if (strcmp(m_szValue, "false") == 0)
					JSONSetBoolean(m_hJSON, m_szKey, false);
				else if (JSONIsNumeric(m_szValue))
					JSONSetInteger(m_hJSON, m_szKey, StringToInt(m_szValue));
				else if ((m_iDot = FindCharInString(m_szValue, '.')) != -1)
				{
					m_szValue[m_iDot] = 0;
					if (JSONIsNumeric(m_szValue) && JSONIsNumeric(m_szValue[m_iDot + 1]))
					{
						m_szValue[m_iDot] = '.';
						JSONSetFloat(m_hJSON, m_szKey, StringToFloat(m_szValue));
					}
				}
			}
		}
	}
	if (len != 0)
		len = m_iPos;

	return m_hJSON;
}

stock Handle DecodeArray(const char[] json, int len = 0)
{
	int m_iPos, m_iIgnore;
	int m_iLength = strlen(json);

	m_iPos = JSONIgnore(json);
	if (m_iPos == -1 || json[m_iPos] != '[')
		return INVALID_HANDLE;
	m_iPos++;
	m_iIgnore = JSONIgnore(json[m_iPos]);
	if (m_iIgnore == -1)
		return INVALID_HANDLE;
	m_iPos += m_iIgnore;
	if (json[m_iPos] == ']')
		return INVALID_HANDLE;

	int m_iStringStart;
	int m_iStringEnd;
	bool m_bEnd = false;
	Handle m_hArray = CreateArray(1);
	while (m_iPos < m_iLength && json[m_iPos] != ']' && !m_bEnd)
	{
		m_iPos += JSONIgnore(json[m_iPos]);
		if (json[m_iPos] == '{')
		{
			int m_iLen = -1;
			Handle m_hObject = DecodeJSON(json[m_iPos], m_iLen);
			if (m_hObject != INVALID_HANDLE)
			{
				PushArrayCell(m_hArray, JSONCreateObject(m_hObject));
				m_iPos += m_iLen;
				m_iPos += JSONIgnore(json[m_iPos]);
			}
			else
			{
				DestroyJSONArray(m_hArray);
				return INVALID_HANDLE;
			}
		}
		if (json[m_iPos] == '[')
		{
			int m_iLen = -1;
			Handle m_hObject = DecodeArray(json[m_iPos], m_iLen);
			if (m_hArray != INVALID_HANDLE)
			{
				PushArrayCell(m_hArray, JSONCreateArray(m_hObject));
				m_iPos += m_iLen;
				m_iPos += JSONIgnore(json[m_iPos]);
			}
			else
			{
				DestroyJSONArray(m_hArray);
				return INVALID_HANDLE;
			}
		}
		else if (GetJSONString(json[m_iPos], m_iStringStart, m_iStringEnd))
		{
			char[] m_szValue = new char[m_iStringEnd - m_iStringStart + 1];
			strcopy(m_szValue, m_iStringEnd-m_iStringStart+1, json[m_iPos+m_iStringStart]);
			//PrintToServer("m_iStringStart%i, m_iStringEnd%i, m_szValue%s%", m_iStringStart, m_iStringEnd, m_szValue);
			if (json[m_iPos + m_iStringEnd] == ']')
				m_bEnd = true;
			m_iPos += m_iStringEnd + 1;
			m_iPos += JSONIgnore(json[m_iPos]);

			int m_iDot = 0;
			if (m_iStringStart != 0)
				PushArrayCell(m_hArray, JSONCreateString(m_szValue));
			else if (strcmp(m_szValue, "true") == 0)
				PushArrayCell(m_hArray, JSONCreateBoolean(true));
			else if (strcmp(m_szValue, "false") == 0)
				PushArrayCell(m_hArray, JSONCreateBoolean(false));
			else if (JSONIsNumeric(m_szValue))
				PushArrayCell(m_hArray, JSONCreateInteger(StringToInt(m_szValue)));
			else if ((m_iDot = FindCharInString(m_szValue, '.')) != -1)
			{
				m_szValue[m_iDot] = 0;
				if (JSONIsNumeric(m_szValue) && JSONIsNumeric(m_szValue[m_iDot + 1]))
				{
					m_szValue[m_iDot] = '.';
					PushArrayCell(m_hArray, JSONCreateFloat(StringToFloat(m_szValue)));
				}
			}
		}
	}
	if (len != 0)
		len = m_iPos + 1;

	return m_hArray;
}

stock void EncodeJSON(Handle json, char[] output, int maxlen, bool beautify = true, int len = 0, int tabs = 1)
{
	Handle m_hKeyArray = INVALID_HANDLE;
	if (!GetTrieValue(json, "json_key_array", m_hKeyArray))
		return;

	int m_iKeys = GetArraySize(m_hKeyArray);

	char m_szKey[512];
	Handle m_hObject;

	if (2 >= maxlen)
		return;

	char[] m_szTabs = new char[tabs + 1];
	if (beautify)
		for (int i = 0; i < tabs; i++)
			m_szTabs[i] = '\t';

	int m_iPos = 1;
	output[0] = '{';
	if (beautify)
	{
		output[1] = '\n';
		m_iPos++;
	}
	for (int i = 0; i < m_iKeys; i++)
	{
		GetArrayString(m_hKeyArray, i, STRING(m_szKey));
		if (!GetTrieValue(json, m_szKey, m_hObject))
			continue;
		JSONType m_eType;
		GetTrieValue(m_hObject, "type", m_eType);
		switch (m_eType)
		{
			case Type_String:
			{
				int m_iLength;
				GetTrieValue(m_hObject, "size", m_iLength);
				char[] m_szValue = new char[m_iLength * 2 + 1];
				GetTrieString(m_hObject, "value", m_szValue, m_iLength+1);
				ReplaceString(m_szValue, m_iLength*2+1, "\"", "\\\""); 
				m_iPos += Format(output[m_iPos], maxlen-m_iPos, "%s\"%s\": \"%s\"", m_szTabs, m_szKey, m_szValue);
			}

			case Type_Boolean:
			{
				bool m_bValue;
				GetTrieValue(m_hObject, "value", m_bValue);
				if (m_bValue)
					m_iPos += Format(output[m_iPos], maxlen-m_iPos, "%s\"%s\": true", m_szTabs, m_szKey);
				else
					m_iPos += Format(output[m_iPos], maxlen-m_iPos, "%s\"%s\": false", m_szTabs, m_szKey);
			} 

			case Type_Integer:
			{
				int m_iValue;
				GetTrieValue(m_hObject, "value", m_iValue);
				m_iPos += Format(output[m_iPos], maxlen-m_iPos, "%s\"%s\": %d", m_szTabs, m_szKey, m_iValue);
			}

			case Type_Float:
			{
				float m_fValue;
				GetTrieValue(m_hObject, "value", m_fValue);
				m_iPos += Format(output[m_iPos], maxlen-m_iPos, "%s\"%s\": %f", m_szTabs, m_szKey, m_fValue);
			}

			case Type_Object:
			{
				Handle m_hValue;
				GetTrieValue(m_hObject, "value", m_hValue);
				m_iPos += Format(output[m_iPos], maxlen-m_iPos, "%s\"%s\": ", m_szTabs, m_szKey);

				int m_iLen = -1;
				EncodeJSON(m_hValue, output[m_iPos], maxlen-m_iPos, beautify, m_iLen, tabs+1);
				m_iPos += m_iLen;
			}

			case Type_Array:
			{
				Handle m_hValue;
				GetTrieValue(m_hObject, "value", m_hValue);
				m_iPos += Format(output[m_iPos], maxlen-m_iPos, "%s\"%s\": ", m_szTabs, m_szKey);

				int m_iLen = -1;
				EncodeArray(m_hValue, output[m_iPos], maxlen-m_iPos, beautify, m_iLen, tabs+1);
				m_iPos += m_iLen+1;
			}

			default:
			{
				continue;
			}
		}
		if (m_iPos >= maxlen)
			return;
		if (m_iKeys != i+1)
		{
			output[m_iPos] = ',';
			++m_iPos;
		}
		if (m_iPos >= maxlen)
			return;
		if (beautify)
		{
			output[m_iPos] = '\n';
			++m_iPos;
		}
	}
	if (m_iPos < maxlen)
	{
		m_szTabs[tabs-1] = 0;
		m_iPos += Format(output[m_iPos], maxlen-m_iPos, "%s}", m_szTabs);
	}
	if (len != 0)
		len=m_iPos;
}

stock void EncodeArray(Handle json, char[] output, int maxlen, bool beautify = true, int &len=0, int tabs = 1)
{
	Handle m_hObject;

	if (2 >= maxlen)
		return;

	char[] m_szTabs = new char[tabs + 1];
	if (beautify)
		for(int i = 0; i < tabs; i++)
			m_szTabs[i] = '\t';

	int m_iPos = 1;
	output[0] = '[';
	if (beautify)
	{
		output[1] = '\n';
		++m_iPos;
	}

	int m_iKeys = GetArraySize(json);
	for (int i = 0; i < m_iKeys; i++)
	{
		m_hObject = GetArrayCell(json, i);

		JSONType m_eType;
		GetTrieValue(m_hObject, "type", m_eType);
		switch (m_eType)
		{
			case Type_String:
			{
				int m_iLength;
				GetTrieValue(m_hObject, "size", m_iLength);
				char[] m_szValue = new char[m_iLength * 2 + 1];
				GetTrieString(m_hObject, "value", m_szValue, m_iLength + 1);
				ReplaceString(m_szValue, m_iLength * 2 + 1, "\"", "\\\""); 
				m_iPos += Format(output[m_iPos], maxlen - m_iPos, "%s\"%s\"", m_szTabs, m_szValue);
			}

			case Type_Boolean:
			{
				bool m_bValue;
				GetTrieValue(m_hObject, "value", m_bValue);
				if (m_bValue)
					m_iPos += Format(output[m_iPos], maxlen - m_iPos, "%strue", m_szTabs);
				else
					m_iPos += Format(output[m_iPos], maxlen - m_iPos, "%sfalse", m_szTabs);
			} 

			case Type_Integer:
			{
				int m_iValue;
				GetTrieValue(m_hObject, "value", m_iValue);
				m_iPos += Format(output[m_iPos], maxlen - m_iPos, "%s%d", m_szTabs, m_iValue);
			}

			case Type_Float:
			{
				float m_fValue;
				GetTrieValue(m_hObject, "value", m_fValue);
				m_iPos += Format(output[m_iPos], maxlen - m_iPos, "%s%f", m_szTabs, m_fValue);
			}

			case Type_Object:
			{
				Handle m_hValue;
				GetTrieValue(m_hObject, "value", m_hValue);
				m_iPos += Format(output[m_iPos], maxlen - m_iPos, "%s", m_szTabs);
				
				int m_iLen = -1;
				EncodeJSON(m_hValue, output[m_iPos], maxlen - m_iPos, beautify, m_iLen, tabs+1);
				m_iPos += m_iLen;
			}

			case Type_Array:
			{
				Handle m_hValue;
				GetTrieValue(m_hObject, "value", m_hValue);
				m_iPos += Format(output[m_iPos], maxlen-m_iPos, "%s", m_szTabs);
				
				int m_iLen = -1;
				EncodeArray(m_hValue, output[m_iPos], maxlen-m_iPos, beautify, m_iLen, tabs+1);
				m_iPos += m_iLen;
			}
		}
		if (m_iPos >= maxlen)
			return;
		if (m_iKeys != i+1)
		{
			output[m_iPos] = ',';
			++m_iPos;
		}
		if (m_iPos >= maxlen)
			return;
		if (beautify)
		{
			output[m_iPos] = '\n';
			++m_iPos;
		}
	}
	if (m_iPos < maxlen)
	{
		m_szTabs[tabs - 1] = 0;
		m_iPos += Format(output[m_iPos], maxlen-m_iPos, "%s]", m_szTabs);
	}
	if (len != 0)
		len=m_iPos - 1;
}

stock void JSONSetString(Handle json, const char[] key, const char[] value)
{
	Handle m_hKeyArray = INVALID_HANDLE;
	if (!GetTrieValue(json, "json_key_array", m_hKeyArray))
		return;
	PushArrayString(m_hKeyArray, key);

	Handle m_hObject = JSONCreateString(value);
	SetTrieValue(json, key, m_hObject);
}

stock void JSONSetBoolean(Handle json, const char[] key, bool value)
{
	Handle m_hKeyArray = INVALID_HANDLE;
	if (!GetTrieValue(json, "json_key_array", m_hKeyArray))
		return;
	PushArrayString(m_hKeyArray, key);

	Handle m_hObject = JSONCreateBoolean(value);
	SetTrieValue(json, key, m_hObject);
}

stock void JSONSetInteger(Handle json, const char[] key, int value)
{
	Handle m_hKeyArray = INVALID_HANDLE;
	if (!GetTrieValue(json, "json_key_array", m_hKeyArray))
		return;
	PushArrayString(m_hKeyArray, key);

	Handle m_hObject = JSONCreateInteger(value);
	SetTrieValue(json, key, m_hObject);
}

stock void JSONSetFloat(Handle json, const char[] key, float value)
{
	Handle m_hKeyArray = INVALID_HANDLE;
	if(!GetTrieValue(json, "json_key_array", m_hKeyArray))
		return;
	PushArrayString(m_hKeyArray, key);

	Handle m_hObject = JSONCreateFloat(value);
	SetTrieValue(json, key, m_hObject);
}

stock void JSONSetObject(Handle json, const char[] key, Handle value)
{
	Handle m_hKeyArray = INVALID_HANDLE;
	if (!GetTrieValue(json, "json_key_array", m_hKeyArray))
		return;
	PushArrayString(m_hKeyArray, key);

	Handle m_hObject = JSONCreateObject(value);
	SetTrieValue(json, key, m_hObject);
}

stock void JSONSetArray(Handle json, const char[] key, Handle value)
{
	Handle m_hKeyArray = INVALID_HANDLE;
	if (!GetTrieValue(json, "json_key_array", m_hKeyArray))
		return;
	PushArrayString(m_hKeyArray, key);

	Handle m_hObject = JSONCreateArray(value);
	SetTrieValue(json, key, m_hObject);
}

stock Handle JSONCreateString(const char[] value)
{
	Handle m_hObject = CreateTrie();
	SetTrieString(m_hObject, "value", value);
	SetTrieValue(m_hObject, "type", Type_String);
	SetTrieValue(m_hObject, "size", strlen(value));
	return m_hObject;
}

stock Handle JSONCreateBoolean(bool value)
{
	Handle m_hObject = CreateTrie();
	SetTrieValue(m_hObject, "value", value);
	SetTrieValue(m_hObject, "type", Type_Boolean);
	SetTrieValue(m_hObject, "size", sizeof(value));
	return m_hObject;
}

stock Handle JSONCreateInteger(int value)
{
	Handle m_hObject = CreateTrie();
	SetTrieValue(m_hObject, "value", value);
	SetTrieValue(m_hObject, "type", Type_Integer);
	SetTrieValue(m_hObject, "size", sizeof(value));
	return m_hObject;
}

stock Handle JSONCreateFloat(float value)
{
	Handle m_hObject = CreateTrie();
	SetTrieValue(m_hObject, "value", value);
	SetTrieValue(m_hObject, "type", Type_Float);
	SetTrieValue(m_hObject, "size", sizeof(value));
	return m_hObject;
}

stock Handle JSONCreateObject(Handle value)
{
	Handle m_hObject = CreateTrie();
	SetTrieValue(m_hObject, "value", value);
	SetTrieValue(m_hObject, "type", Type_Object);
	SetTrieValue(m_hObject, "size", sizeof(value));
	return m_hObject;
}

stock Handle JSONCreateArray(Handle value)
{
	Handle m_hObject = CreateTrie();
	SetTrieValue(m_hObject, "value", value);
	SetTrieValue(m_hObject, "type", Type_Array);
	SetTrieValue(m_hObject, "size", sizeof(value));
	return m_hObject;
}

stock bool JSONGetString(Handle json, const char[] key, char[] out, int maxlen)
{
	Handle m_hObject;
	if (!GetTrieValue(json, key, m_hObject))
		return false;
	JSONType m_eType;
	GetTrieValue(m_hObject, "type", m_eType);
	if (m_eType != Type_String)
		return false;
	GetTrieString(m_hObject, "value", out, maxlen);
	return true;
}

stock bool JSONGetObject(Handle json, const char[] key, Handle out)
{
	Handle m_hObject;
	if (!GetTrieValue(json, key, m_hObject))
		return false;
	JSONType m_eType;
	GetTrieValue(m_hObject, "type", m_eType);
	if (m_eType != Type_Object)
		return false;
	GetTrieValue(m_hObject, "value", out);
	return true;
}

stock bool JSONGetArray(Handle json, const char[] key, Handle out)
{
	Handle m_hObject;
	if (!GetTrieValue(json, key, m_hObject))
		return false;
	JSONType m_eType;
	GetTrieValue(m_hObject, "type", m_eType);
	if (m_eType != Type_Array)
		return false;
	GetTrieValue(m_hObject, "value", out);
	return true;
}

stock bool JSONGetBoolean(Handle json, const char[] key, bool value)
{
	Handle m_hObject;
	if (!GetTrieValue(json, key, m_hObject))
		return false;
	JSONType m_eType;
	GetTrieValue(m_hObject, "type", m_eType);
	if (m_eType != Type_Boolean)
		return false;
	GetTrieValue(m_hObject, "value", value);
	return true;
}

stock bool JSONGetInteger(Handle json, const char[] key, int &value)
{
	Handle m_hObject;
	if (!GetTrieValue(json, key, m_hObject))
		return false;
	JSONType m_eType;
	GetTrieValue(m_hObject, "type", m_eType);
	if (m_eType != Type_Integer)
		return false;
	GetTrieValue(m_hObject, "value", value);
	return true;
}

stock bool JSONGetFloat(Handle json, const char[] key, float &value)
{
	Handle m_hObject;
	if (!GetTrieValue(json, key, m_hObject))
		return false;
	JSONType m_eType;
	GetTrieValue(m_hObject, "type", m_eType);
	// For compatibility reasons only, but you should follow strict types
	if (m_eType != Type_Float && m_eType != Type_Integer)
		return false;
	if (m_eType == Type_Integer)
	{
		int m_iValue;
		GetTrieValue(m_hObject, "value", m_iValue);
		value = float(m_iValue);
	}
	else
		GetTrieValue(m_hObject, "value", value);
	return true;
}

stock bool JSONGetArrayString(Handle array, int idx, char[] out, int maxlen)
{
	Handle m_hObject = GetArrayCell(array, idx);
	JSONType m_eType;
	GetTrieValue(m_hObject, "type", m_eType);
	if (m_eType != Type_String)
		return false;
	GetTrieString(m_hObject, "value", out, maxlen);
	return true;
}

stock bool JSONGetArrayObject(Handle array, int idx, Handle out)
{
	Handle m_hObject = GetArrayCell(array, idx);
	JSONType m_eType;
	GetTrieValue(m_hObject, "type", m_eType);
	if (m_eType != Type_Object)
		return false;
	GetTrieValue(m_hObject, "value", out);
	return true;
}

stock bool JSONGetArrayArray(Handle array, int idx, Handle out)
{
	Handle m_hObject = GetArrayCell(array, idx);
	JSONType m_eType;
	GetTrieValue(m_hObject, "type", m_eType);
	if (m_eType != Type_Array)
		return false;
	GetTrieValue(m_hObject, "value", out);
	return true;
}

stock bool JSONGetArrayBoolean(Handle array, int idx, bool value)
{
	Handle m_hObject = GetArrayCell(array, idx);
	JSONType m_eType;
	GetTrieValue(m_hObject, "type", m_eType);
	if (m_eType != Type_Boolean)
		return false;
	GetTrieValue(m_hObject, "value", value);
	return true;
}

stock bool JSONGetArrayInteger(Handle array, int idx, int &value)
{
	Handle m_hObject = GetArrayCell(array, idx);
	JSONType m_eType;
	GetTrieValue(m_hObject, "type", m_eType);
	if (m_eType != Type_Integer)
		return false;
	GetTrieValue(m_hObject, "value", value);
	return true;
}

stock bool JSONGetArrayFloat(Handle array, int idx, float value)
{
	Handle m_hObject = GetArrayCell(array, idx);
	JSONType m_eType;
	GetTrieValue(m_hObject, "type", m_eType);
	// For compatibility reasons only, but you should follow strict types
	if(m_eType != Type_Float && m_eType != Type_Integer)
		return false;
	if (m_eType == Type_Integer)
	{
		int m_iValue;
		GetTrieValue(m_hObject, "value", m_iValue);
		value = float(m_iValue);
	}
	else
		GetTrieValue(m_hObject, "value", value);
	return true;
}

stock int JSONIgnore(const char[] json)
{
	int m_iLength = strlen(json);
	for (int i = 0; i < m_iLength; i++)
		if (json[i] != ' ' && json[i] != '\t' && json[i] != ':' && json[i] != '\n' && json[i] != '\r' && json[i] != ',')
			return i;
	return -1;
}

stock bool JSONIsNumeric(const char[] string)
{
	int m_iLength = strlen(string);
	int i = 0;
	if (string[0] == '-')
		i = 1;
	for (; i < m_iLength; i++)
		if (!(48<=string[i]<=57))
			return false;
	return true;
}

stock bool GetJSONString(const char[] json, int &start, int &end)
{
	int m_iCharPos;
	int m_iPos = JSONIgnore(json);
	start = m_iPos == -1 ? 0 : m_iPos;
	if (json[m_iPos] == '"')
	{
		start += 1;
		m_iPos += 1;
		while ((m_iCharPos = FindCharInString(json[m_iPos], '"')) != -1)
		{
			m_iPos += m_iCharPos;
			if (json[m_iPos-1] == '\\')
				continue;
			end = m_iPos;
			return true;
		}
	}

	int m_iLength = strlen(json);
	for (; m_iPos < m_iLength; m_iPos++)
	{
		if (m_iPos != 0 && !(48 <= json[m_iPos] <= 57) && !(97 <= json[m_iPos] <= 122) && json[m_iPos] != '-' && json[m_iPos] != '.')
		{
			end = m_iPos;
			return true;
		}
	}
	
	return false;
}

stock void DestroyJSON(Handle json)
{
	Handle m_hKeyArray = INVALID_HANDLE;
	if (!GetTrieValue(json, "json_key_array", m_hKeyArray))
	{
		CloseHandle(json);
		return;
	}

	int m_iKeys = GetArraySize(m_hKeyArray);
	char m_szKey[512];
	Handle m_hObject;
	for (int i = 0; i < m_iKeys; i++)
	{
		GetArrayString(m_hKeyArray, i, STRING(m_szKey));
		if (!GetTrieValue(json, m_szKey, m_hObject))
			continue;
		JSONType m_eType;
		GetTrieValue(m_hObject, "type", m_eType);
		if (m_eType == Type_Object)
		{
			Handle m_hValue;
			GetTrieValue(m_hObject, "value", m_hValue);
			DestroyJSON(m_hValue);
		}
		else if (m_eType == Type_Array)
		{
			Handle m_hValue;
			GetTrieValue(m_hObject, "value", m_hValue);
			DestroyJSONArray(m_hValue);
		}
		else
			CloseHandle(m_hObject);
	}
	
	CloseHandle(m_hKeyArray);
	CloseHandle(json);
}

stock void DestroyJSONArray(Handle array)
{
	int m_iLength = GetArraySize(array);
	for (int i = 0; i < m_iLength; i++)
	{
		Handle m_hItem;
		m_hItem = GetArrayCell(array, i);

		JSONType m_eType;
		GetTrieValue(m_hItem, "type", m_eType);
		if (m_eType == Type_Object)
		{
			Handle m_hValue;
			GetTrieValue(m_hItem, "value", m_hValue);
			DestroyJSON(m_hValue);
		}
		else if(m_eType == Type_Array)
		{
			Handle m_hValue;
			GetTrieValue(m_hItem, "value", m_hValue);
			DestroyJSONArray(m_hValue);
		}
		else
			CloseHandle(m_hItem);
	}
	CloseHandle(array);
}