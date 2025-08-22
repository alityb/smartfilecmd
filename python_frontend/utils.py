#!/usr/bin/env python3
"""
Utility functions for SmartFileCmd Python frontend.
"""

import json
import subprocess
import os
from pathlib import Path
from typing import Dict, Any, Optional

def validate_command(command: Dict[str, Any]) -> bool:
    """Validate command structure."""
    required_fields = ['action']
    
    for field in required_fields:
        if field not in command:
            return False
    
    action = command['action']
    
    if action in ['move', 'copy']:
        return 'source' in command and 'destination' in command
    elif action == 'delete':
        return 'source' in command
    elif action == 'create_folder':
        return 'destination' in command
    else:
        return False

def send_command_to_backend(command: Dict[str, Any]) -> Optional[Dict[str, Any]]:
    """Send command to C++ backend and return result."""
    try:
        # Get backend path
        backend_path = get_cpp_backend_path()
        
        # Convert command to JSON
        command_json = json.dumps(command)
        
        # Debug: print the JSON being sent
        print(f"DEBUG: Sending JSON: {command_json}")
        
        # Send to backend
        result = subprocess.run(
            [backend_path],
            input=command_json,
            text=True,
            capture_output=True,
            timeout=300  # 5 minute timeout
        )
        
        if result.returncode != 0:
            print(f"Backend error: {result.stderr}")
            return None
        
        # Parse result
        try:
            return json.loads(result.stdout)
        except json.JSONDecodeError:
            print(f"Failed to parse backend output: {result.stdout}")
            return None
            
    except subprocess.TimeoutExpired:
        print("Backend operation timed out")
        return None
    except Exception as e:
        print(f"Failed to communicate with backend: {e}")
        return None

def get_cpp_backend_path() -> str:
    """Get path to C++ backend executable."""
    # Check build directory first
    build_path = Path("build/smartfilecmd")
    if build_path.exists():
        return str(build_path.absolute())
    
    # Check current directory
    current_path = Path("smartfilecmd")
    if current_path.exists():
        return str(current_path.absolute())
    
    # Check system PATH
    try:
        result = subprocess.run(["which", "smartfilecmd"], capture_output=True, text=True)
        if result.returncode == 0:
            return result.stdout.strip()
    except:
        pass
    
    raise FileNotFoundError("C++ backend executable not found. Please build the project first.")

def format_result(result: Dict[str, Any], verbose: bool = False) -> str:
    """Format operation result for display."""
    if not result.get('success', False):
        error_msg = result.get('error_message', 'Unknown error')
        return f"❌ Operation failed: {error_msg}"
    
    # Basic result info
    operation = result.get('operation', 'unknown')
    message = result.get('message', 'Operation completed')
    files_scanned = result.get('files_scanned', 0)
    files_matched = result.get('files_matched', 0)
    files_affected = result.get('files_affected', 0)
    
    # Calculate duration
    duration_ms = 0
    if 'start_time' in result and 'end_time' in result:
        start = result['start_time']
        end = result['end_time']
        if isinstance(start, str) and isinstance(end, str):
            try:
                import time
                start_time = time.mktime(time.strptime(start, "%Y-%m-%d %H:%M:%S"))
                end_time = time.mktime(time.strptime(end, "%Y-%m-%d %H:%M:%S"))
                duration_ms = int((end_time - start_time) * 1000)
            except:
                pass
    
    # Build output
    output = []
    
    if verbose:
        output.append(f"🔍 Operation: {operation}")
        output.append(f"📊 Files scanned: {files_scanned}")
        output.append(f"🎯 Files matched: {files_matched}")
        output.append(f"⚡ Files affected: {files_affected}")
        if duration_ms > 0:
            output.append(f"⏱️ Duration: {duration_ms}ms")
    
    output.append(f"✅ {message}")
    
    # Show errors if any
    errors = result.get('errors', [])
    if errors and verbose:
        output.append("\n⚠️ Errors encountered:")
        for error in errors:
            output.append(f"  • {error}")
    
    return "\n".join(output)

def confirm_destructive_operation(command: Dict[str, Any]) -> bool:
    """Ask user to confirm destructive operations."""
    action = command.get('action', '')
    source = command.get('source', '')
    
    if action in ['delete']:
        print(f"\n⚠️  WARNING: This will DELETE files from '{source}'")
        response = input("Are you sure you want to continue? (yes/no): ").lower().strip()
        return response in ['yes', 'y']
    
    return True

def expand_path(path: str) -> str:
    """Expand user paths and environment variables."""
    expanded = os.path.expanduser(os.path.expandvars(path))
    return str(Path(expanded).resolve())

def format_dry_run(command: Dict[str, Any]) -> str:
    """Format command for dry-run display."""
    action = command.get('action', '')
    source = command.get('source', '')
    destination = command.get('destination', '')
    pattern = command.get('pattern', '')
    
    if action == 'delete':
        return f"Would delete files matching '{pattern}' from '{source}'"
    elif action in ['move', 'copy']:
        return f"Would {action} files matching '{pattern}' from '{source}' to '{destination}'"
    elif action == 'create_folder':
        return f"Would create folder '{destination}'"
    else:
        return f"Would execute {action} operation"
