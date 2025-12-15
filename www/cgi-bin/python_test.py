#!/usr/bin/env python3
# Python CGI script for dynamic content

import os
import datetime
import html

print("Content-Type: text/html; charset=UTF-8")
print()

print("""
<html>
<head>
    <meta charset="UTF-8">
    <title>Python CGI Test</title>
    <style>
        body { font-family: Arial, sans-serif; margin: 20px; }
        .info-box { background: #e7f3ff; border: 1px solid #bee5eb; padding: 15px; margin: 10px 0; border-radius: 5px; }
        .error-box { background: #f8d7da; border: 1px solid #f5c6cb; padding: 15px; margin: 10px 0; border-radius: 5px; }
        table { border-collapse: collapse; width: 100%; }
        th, td { border: 1px solid #ddd; padding: 8px; text-align: left; }
        th { background-color: #f2f2f2; }
    </style>
</head>
<body>
    <h1>🐍 Python CGI Script</h1>
""")

# Display current time
now = datetime.datetime.now()
print(f"<div class='info-box'>")
print(f"<h2>⏰ Current Server Time</h2>")
print(f"<p><strong>Date:</strong> {now.strftime('%Y-%m-%d')}</p>")
print(f"<p><strong>Time:</strong> {now.strftime('%H:%M:%S')}</p>")
print(f"<p><strong>Timezone:</strong> {now.astimezone().tzname()}</p>")
print(f"</div>")

# Display environment variables
print("<h2>🌐 CGI Environment</h2>")
print("<table>")
print("<tr><th>Variable</th><th>Value</th></tr>")

cgi_vars = [
    'REQUEST_METHOD', 'REQUEST_URI', 'QUERY_STRING', 'SCRIPT_NAME',
    'HTTP_HOST', 'HTTP_USER_AGENT', 'CONTENT_TYPE', 'CONTENT_LENGTH'
]

for var in cgi_vars:
    value = os.environ.get(var, 'Not set')
    print(f"<tr><td><strong>{html.escape(var)}</strong></td><td>{html.escape(value)}</td></tr>")

print("</table>")

# Parse query string
query_string = os.environ.get('QUERY_STRING', '')
if query_string:
    print("<h2>🔍 Query Parameters</h2>")
    print("<table>")
    print("<tr><th>Parameter</th><th>Value</th></tr>")
    
    for pair in query_string.split('&'):
        if '=' in pair:
            key, value = pair.split('=', 1)
            # Basic URL decoding
            value = value.replace('+', ' ').replace('%20', ' ')
            print(f"<tr><td>{html.escape(key)}</td><td>{html.escape(value)}</td></tr>")
    print("</table>")

# Simple calculator
print("""
<h2>🧮 Simple Calculator</h2>
<form method="GET" action="/cgi-bin/python_test.py">
    <p>
        <input type="number" name="a" placeholder="First number" value="">
        <select name="op">
            <option value="+">+</option>
            <option value="-">-</option>
            <option value="*">×</option>
            <option value="/">/</option>
        </select>
        <input type="number" name="b" placeholder="Second number" value="">
        <input type="submit" value="Calculate">
    </p>
</form>
""")

# Process calculation if parameters provided
a = os.environ.get('QUERY_STRING', '').find('a=')
b = os.environ.get('QUERY_STRING', '').find('b=')
op = os.environ.get('QUERY_STRING', '').find('op=')

if a != -1 and b != -1 and op != -1:
    try:
        # Extract values (simple parsing)
        query_params = {}
        for pair in query_string.split('&'):
            if '=' in pair:
                key, value = pair.split('=', 1)
                query_params[key] = value
        
        num_a = float(query_params.get('a', '0'))
        num_b = float(query_params.get('b', '0'))
        operation = query_params.get('op', '+')
        
        if operation == '+':
            result = num_a + num_b
        elif operation == '-':
            result = num_a - num_b
        elif operation == '*':
            result = num_a * num_b
        elif operation == '/':
            result = num_a / num_b if num_b != 0 else 'Error: Division by zero'
        else:
            result = 'Error: Unknown operation'
        
        print(f"<div class='info-box'>")
        print(f"<h3>📊 Calculation Result</h3>")
        print(f"<p><strong>{num_a} {operation} {num_b} = {result}</strong></p>")
        print(f"</div>")
        
    except Exception as e:
        print(f"<div class='error-box'>")
        print(f"<h3>❌ Calculation Error</h3>")
        print(f"<p>Error: {html.escape(str(e))}</p>")
        print(f"</div>")

print("""
    <h2>🔗 Other CGI Tests</h2>
    <ul>
        <li><a href="/cgi-bin/test.cgi">Basic CGI Test</a></li>
        <li><a href="/cgi-bin/info.cgi">Environment Info</a></li>
        <li><a href="/cgi-bin/form.cgi">Form Handler</a></li>
        <li><a href="/cgi-bin/upload.cgi">File Upload Test</a></li>
    </ul>
</body>
</html>
""")
