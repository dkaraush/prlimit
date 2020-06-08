#include <nan.h>
#include <sys/resource.h>
#include <limits>
#include <cctype>
#include <iostream>

using namespace v8;

Local<Value> RLimitToV8Value(rlim_t limit) {
	if (limit == RLIM_INFINITY) {
		return Nan::New<Number>(std::numeric_limits<double>::infinity());
	} else if (limit == RLIM_SAVED_MAX || limit == RLIM_SAVED_CUR) {
		return Nan::Null();
	} else {
		return Nan::New<Number>((double) limit);
	}
}

rlim_t V8ValueToRLimit(Local<Value> value, bool isCur) {
	if (value->IsNull()) {
		return (isCur ? RLIM_SAVED_CUR : RLIM_SAVED_MAX);
	} else if (value->IsNumber()) {
		if (Nan::To<double>(value).FromJust() == std::numeric_limits<double>::infinity())
			return RLIM_INFINITY;
		return Nan::To<uint>(value).FromJust();
	}
}

Local<String> V8String(const char* char_array) {
	return Nan::New<String>(char_array).ToLocalChecked();
}

struct Limit {
	const char* name;
	__rlimit_resource resource;
};

const Limit limits[] = {
	#ifdef RLIMIT_AS
		{ "as", RLIMIT_AS },
	#endif
	#ifdef RLIMIT_CORE
		{ "core", RLIMIT_CORE },
	#endif
	#ifdef RLIMIT_CPU
		{ "cpu", RLIMIT_CPU },
	#endif
	#ifdef RLIMIT_DATA
		{ "data", RLIMIT_DATA },
	#endif
	#ifdef RLIMIT_FSIZE
		{ "fsize", RLIMIT_FSIZE },
	#endif
	#ifdef RLIMIT_LOCKS
		{ "locks", RLIMIT_LOCKS },
	#endif
	#ifdef RLIMIT_MEMLOCK
		{ "memlock", RLIMIT_MEMLOCK },
	#endif
	#ifdef RLIMIT_MSGQUEUE
		{ "msgqueue", RLIMIT_MSGQUEUE },
	#endif
	#ifdef RLIMIT_NICE
		{ "nice", RLIMIT_NICE },
	#endif
	#ifdef RLIMIT_NOFILE
		{ "nofile", RLIMIT_NOFILE },
	#endif
	#ifdef RLIMIT_NPROC
		{ "nproc", RLIMIT_NPROC },
	#endif
	#ifdef RLIMIT_RSS
		{ "rss", RLIMIT_RSS },
	#endif
	#ifdef RLIMIT_RTPRIO
		{ "rtprio", RLIMIT_RTPRIO },
	#endif
	#ifdef RLIMIT_RTTIME
		{ "rttime", RLIMIT_RTTIME },
	#endif
	#ifdef RLIMIT_SIGPENDING
		{ "sigpending", RLIMIT_SIGPENDING },
	#endif
	#ifdef RLIMIT_STACK
		{ "stack", RLIMIT_STACK },
	#endif
	{ 0, static_cast<__rlimit_resource>(0) }
};

NAN_METHOD(prlimit) {
	if (info.Length() < 2 || info.Length() > 3)
		return Nan::ThrowError("prlimit: function takes 2-3 arguments");

	if (!info[0]->IsNumber())
		return Nan::ThrowError("prlimit: first argument must be a number (pid)");
	pid_t pid = Nan::To<signed int>(info[0]).FromJust();

	bool found_resource = false;
	__rlimit_resource resource;
	if (info[1]->IsNumber()) {
		resource = static_cast<__rlimit_resource>(Nan::To<int>(info[1]).FromJust());
		found_resource = true;
	} else if (info[1]->IsString()) {
		Nan::Utf8String resource_string(info[1]);
		// to lower case
		unsigned char *resource_string_ptr = (unsigned char *) &resource_string;
		while (*resource_string_ptr) {
			*resource_string_ptr = std::tolower(*resource_string_ptr);
			resource_string_ptr++;
		}
		for (const Limit* item = limits; item->name; ++item) {
			if (strcmp(*resource_string, item->name) == 0) {
				resource = item->resource;
				found_resource = true;
				break;
			}
		}

		if (!found_resource)
			return Nan::ThrowError("prlimit: resource is not found by string (could be not supported in your OS)");
	} else {
		return Nan::ThrowError("prlimit: second argument must be a number or a string (resource)");
	}

	bool has_new_limit = false;
	struct rlimit new_limit;
	if (info.Length() == 3) {
		Local<Object> new_limit_obj = Nan::To<v8::Object>(info[2]).ToLocalChecked();
		new_limit = rlimit{
			.rlim_cur = V8ValueToRLimit(new_limit_obj->Get(V8String("soft")), true),
			.rlim_max = V8ValueToRLimit(new_limit_obj->Get(V8String("hard")), false)
		};
		has_new_limit = true;
	}

	struct rlimit old_limit;

	int err = prlimit(pid, resource, has_new_limit ? &new_limit : NULL, &old_limit);

	if (err == EFAULT)
		return Nan::ThrowError("prlimit: EFAULT: A pointer argument points to a location outside the accessible address space.");
	if (err == EINVAL)
		return Nan::ThrowError("prlimit: EINVAL: The value specified in resource is not valid; or, for setrlimit() or prlimit(): rlim->rlim_cur was greater than rlim->rlim_max.");
	if (err == EPERM)
		return Nan::ThrowError("prlimit: EPERM: An unprivileged process tried to raise the hard limit; the CAP_SYS_RESOURCE capability is required to do this. Or, the caller tried to increase the hard RLIMIT_NOFILE limit above the current kernel maximum (NR_OPEN). Or, the calling process did not have permission to set limits for the process specified by pid.");
	if (err == ESRCH)
		return Nan::ThrowError("prlimit: ESRCH: Could not find a process with the ID specified in pid.");
	if (err)
		return Nan::ThrowError("prlimit: Unknown error number: " + err);

	Local<Object> old_limit_obj = Nan::New<Object>();
	old_limit_obj->Set(V8String("soft"), RLimitToV8Value(old_limit.rlim_cur));
	old_limit_obj->Set(V8String("hard"), RLimitToV8Value(old_limit.rlim_max));

	info.GetReturnValue().Set(old_limit_obj);
}

void init(Handle<Object> exports, Handle<Object> module) {
	Nan::Set(
		module,
		V8String("exports"),
		Nan::GetFunction(Nan::New<FunctionTemplate>(prlimit)).ToLocalChecked()
	);
}

NODE_MODULE(prlimit, init);
