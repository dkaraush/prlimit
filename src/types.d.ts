declare module "node-prlimit" {
	export interface Limit {
		hard: number | null;
		soft: number | null;
	}

	export type ResourceLimit = "cpu" | "data" | "fsize" | "locks" |
		"memlock" | "msgqueue" | "nice" | "nofile" |
		"nproc" | "rss" | "rtprio" | "rttime" | "sigpending";

	function prlimit (
		pid: number,
		resource: number | ResourceLimit,
		new_limit: Limit
	) : Limit;

	export = prlimit;
}
