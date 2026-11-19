"""Single-purpose stdio MCP publishing tool for an isolated documentation pilot."""
import base64
import json
import os
from pathlib import Path
import re
import subprocess
import sys
import urllib.error
import urllib.request

ROOT = Path(os.environ['DOC_WORKSPACE']).resolve()
REPO = os.environ['GITHUB_REPOSITORY']
TASK = os.environ['DOC_TASK_ID']
BRANCH = os.environ['DOC_BRANCH']
BASE = 'integration/claude-bedrock-cloud-20261119'
TARGET = 'docs/cloud-agent-validation-' + TASK + '.md'
TOKEN = os.environ['GH_TOKEN']
RESULT = Path(os.environ['RUNNER_TEMP']) / 'publish-result.json'
BASE_SHA = os.environ['DOC_BASE_SHA']
assert REPO in ('nexvary/NEXVARY-MR', 'Erthi-Org/Muslim-Guide')
assert re.fullmatch(r'[a-z0-9-]{1,40}', TASK)
assert BRANCH == 'claude/cloud-doc-' + TASK
attempted = False

def api(method, path, data=None):
    req = urllib.request.Request('https://api.github.com/repos/' + REPO + path,
        data=None if data is None else json.dumps(data).encode(), method=method,
        headers={'Authorization': 'Bearer ' + TOKEN, 'Accept': 'application/vnd.github+json',
                 'Content-Type': 'application/json', 'X-GitHub-Api-Version': '2022-11-28'})
    try:
        with urllib.request.urlopen(req, timeout=30) as response:
            return json.load(response)
    except urllib.error.HTTPError as exc:
        try:
            message = json.loads(exc.read()).get('message', 'GitHub request rejected')
        except Exception:
            message = 'GitHub request rejected'
        raise RuntimeError('GitHub HTTP ' + str(exc.code) + ': ' + message) from None

def git(*args, authenticated=False):
    env = {k: v for k, v in os.environ.items() if not k.startswith(('GIT_', 'AWS_', 'ACTIONS_'))}
    env['GIT_CONFIG_NOSYSTEM'] = '1'
    env['GIT_CONFIG_GLOBAL'] = '/dev/null'
    env['GIT_TERMINAL_PROMPT'] = '0'
    cmd = ['git', '-c', 'core.hooksPath=/dev/null', '-c', 'commit.gpgsign=false',
           '-c', 'user.name=claude[bot]', '-c', 'user.email=41898282+claude[bot]@users.noreply.github.com']
    if authenticated:
        auth = base64.b64encode(('x-access-token:' + TOKEN).encode()).decode()
        cmd += ['-c', 'http.https://github.com/.extraheader=AUTHORIZATION: basic ' + auth]
    proc = subprocess.run(cmd + list(args), cwd=ROOT, env=env, text=True,
                          stdout=subprocess.PIPE, stderr=subprocess.PIPE, timeout=60)
    if proc.returncode:
        # Do not return raw subprocess diagnostics that might contain authentication data.
        raise RuntimeError('Controlled git operation failed with exit ' + str(proc.returncode))
    return proc.stdout.strip()

def publish():
    global attempted
    if attempted:
        raise RuntimeError('Publication already attempted; inspect run evidence before any retry')
    attempted = True
    assert git('branch', '--show-current') == BRANCH, 'Unexpected branch'
    assert git('rev-parse', 'HEAD') == BASE_SHA, 'Unexpected pre-existing commit'
    target = ROOT / TARGET
    for path in (ROOT / 'docs', target, ROOT / '.git'):
        assert not path.is_symlink(), 'Symlinks are forbidden'
    assert target.resolve().is_relative_to(ROOT), 'Path escaped workspace'
    content = target.read_text('utf-8')
    assert 30 <= len(content.encode()) <= 8192, 'Document size outside pilot limits'
    assert not re.search(r'AKIA[A-Z0-9]{16}|-----BEGIN .*PRIVATE KEY|gh[pousr]_[A-Za-z0-9]{20,}', content), 'Possible secret'
    status = git('status', '--porcelain', '--untracked-files=all').splitlines()
    assert status == ['?? ' + TARGET], 'Only the newly requested document may change'
    git('add', '--', TARGET)
    git('commit', '-m', 'docs: Claude cloud development validation ' + TASK)
    sha = git('rev-parse', 'HEAD')
    assert git('diff', '--name-only', BASE_SHA, sha).splitlines() == [TARGET]
    assert git('remote', 'get-url', 'origin') == 'https://github.com/' + REPO, 'Unexpected remote'
    git('push', 'origin', 'HEAD:refs/heads/' + BRANCH, authenticated=True)
    data = {'task': TASK, 'branch': BRANCH, 'commit': sha, 'document': TARGET, 'pushed': True}
    RESULT.write_text(json.dumps(data), 'utf-8')
    pr = api('POST', '/pulls', {'title': 'docs: Claude cloud development validation ' + TASK,
        'head': BRANCH, 'base': BASE, 'draft': True,
        'body': 'Created by Claude Code through Amazon Bedrock on a GitHub-hosted runner.\n\n'
                'Documentation-only validation; no application changes.\n'
                'Task: ' + TASK + '\nWorkflow: https://github.com/' + REPO + '/actions/runs/' + os.environ['GITHUB_RUN_ID']})
    data.update({'pr_url': pr['html_url'], 'pr_number': pr['number'], 'draft': pr['draft']})
    RESULT.write_text(json.dumps(data), 'utf-8')
    print('Claude requested controlled publication: ' + json.dumps(data), file=sys.stderr)
    return data

def respond(identifier, result=None, error=None):
    obj = {'jsonrpc': '2.0', 'id': identifier}
    obj['error' if error else 'result'] = error if error else result
    print(json.dumps(obj), flush=True)

for line in sys.stdin:
    request = json.loads(line)
    if 'id' not in request:
        continue
    method = request.get('method')
    if method == 'initialize':
        respond(request['id'], {'protocolVersion': '2024-11-05', 'capabilities': {'tools': {}},
            'serverInfo': {'name': 'controlled-doc-publication', 'version': '1.0.0'}})
    elif method == 'ping':
        respond(request['id'], {})
    elif method == 'tools/list':
        respond(request['id'], {'tools': [
            {'name': 'project_context', 'description': 'Read project identity/instructions and assigned task; no arbitrary file access.',
             'inputSchema': {'type': 'object', 'properties': {}, 'additionalProperties': False}},
            {'name': 'write_document', 'description': 'Create the single assigned Markdown document. No arbitrary paths or code execution.',
             'inputSchema': {'type': 'object', 'properties': {'content': {'type': 'string', 'minLength': 30, 'maxLength': 8192}}, 'required': ['content'], 'additionalProperties': False}},
            {'name': 'publish_document',
            'description': 'After writing the requested documentation file, commit it, push the assigned feature branch and open a Draft PR. Enforces exact file and branch; no shell, merges or main access. Call once only.',
            'inputSchema': {'type': 'object', 'properties': {}, 'additionalProperties': False}}]})
    elif method == 'tools/call':
        try:
            name = request.get('params', {}).get('name')
            args = request.get('params', {}).get('arguments', {})
            if name == 'project_context':
                assert not args
                result = {'repository': REPO, 'task_id': TASK, 'target': TARGET,
                          'branch': BRANCH, 'base_sha': BASE_SHA, 'instructions': {}}
                for filename in ['AGENTS.md', 'README.md']:
                    path = ROOT / filename
                    if path.is_file() and not path.is_symlink():
                        result['instructions'][filename] = path.read_text('utf-8')[:5000]
            elif name == 'write_document':
                assert set(args) == {'content'} and isinstance(args['content'], str)
                assert 30 <= len(args['content'].encode()) <= 8192
                parent = ROOT / 'docs'
                assert not parent.is_symlink()
                parent.mkdir(exist_ok=True)
                with (ROOT / TARGET).open('x', encoding='utf-8') as output:
                    output.write(args['content'])
                result = {'written': TARGET, 'bytes': len(args['content'].encode())}
            else:
                assert name == 'publish_document', 'Unknown tool'
                assert not args, 'Unexpected arguments'
                result = publish()
            respond(request['id'], {'content': [{'type': 'text', 'text': json.dumps(result)}]})
        except Exception as exc:
            respond(request['id'], {'isError': True, 'content': [{'type': 'text', 'text': str(exc)[:500]}]})
    else:
        respond(request['id'], error={'code': -32601, 'message': 'Method not found'})
