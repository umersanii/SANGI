"""Shared GitHub API client used by both notification and stats monitors."""

import logging
import requests


class GitHubAPI:
    BASE_URL = "https://api.github.com"
    GRAPHQL_URL = "https://api.github.com/graphql"

    def __init__(self, token: str):
        self.session = requests.Session()
        self.session.headers.update({
            "Authorization": f"token {token}",
            "Accept": "application/vnd.github.v3+json",
        })
        self.logger = logging.getLogger("GitHubAPI")

    def get(self, path: str, **kwargs) -> requests.Response:
        url = f"{self.BASE_URL}{path}"
        resp = self.session.get(url, timeout=30, **kwargs)
        resp.raise_for_status()
        return resp

    def graphql(self, query: str, variables: dict = None) -> dict:
        payload = {"query": query}
        if variables:
            payload["variables"] = variables
        # GraphQL needs bearer auth
        headers = {"Authorization": self.session.headers["Authorization"].replace("token ", "bearer ")}
        resp = self.session.post(self.GRAPHQL_URL, json=payload, timeout=30, headers=headers)
        resp.raise_for_status()
        return resp.json()
