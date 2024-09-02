import { useCallback, useEffect, useRef, useState } from 'preact/hooks'
import sunFilledIcon from '@assets/sun-filled.svg?raw'
import sunIcon from '@assets/sun.svg?raw'
import Icon from '@components/Icon'
import { useToast } from '@/hooks/toast'
import { useAuth } from '@/hooks/auth'

interface FanStatusData {
	light: boolean
	fan: boolean
	speed: number
}

export function FanControl() {
	const { addToast } = useToast()
	const { user } = useAuth()
	const [fanState, setFanState] = useState<FanStatusData | null>(null)

	const ws = useRef<WebSocket | null>(null)

	const getStatus = async () => {
		const response = await fetch('/api/status')

		if (!response.ok) {
			return
		}

		const data = (await response.json()) as FanStatusData

		setFanState(data)
	}

	const handleLightButtonClick = useCallback(async () => {
		if (!user) {
			addToast({
				description: 'This action requires authentication',
				type: 'error'
			})
			return
		}

		const response = await fetch('/api/light/toggle')

		if (!response.ok) {
			addToast({
				description: 'You do not have permission to perform this action',
				type: 'error'
			})
			return
		}

		const data = (await response.json()) as { value: boolean }

		setFanState((old) => (old ? { ...old, light: data.value } : null))
	}, [user])

	const handleFanButtonClick = useCallback(async () => {
		if (!user) {
			addToast({
				description: 'This action requires authentication',
				type: 'error'
			})
			return
		}

		const response = await fetch('/api/fan/toggle')

		if (!response.ok) {
			addToast({
				description: 'You do not have permission to perform this action',
				type: 'error'
			})
			return
		}

		const data = (await response.json()) as { value: boolean }

		setFanState((old) => (old ? { ...old, fan: data.value } : null))
	}, [user])

	useEffect(() => {
		getStatus()
		ws.current = new WebSocket(`ws://${location.host}/ws`)
		ws.current.onmessage = (ev) => {
			const rawData = ev.data
			const data = JSON.parse(rawData) as FanStatusData
			setFanState(data)
		}
	}, [])

	return (
		<div class="container py-8 flex flex-col justify-center items-center">
			<div class="flex gap-2">
				<button
					type="button"
					class="btn btn-lg btn-circle col-start-1 group data-[active='true']:btn-active"
					onClick={handleLightButtonClick}
					data-active={!!fanState?.light}
				>
					<Icon
						class="group-data-[active='false']:hidden"
						icon={sunFilledIcon}
					/>
					<Icon class="group-data-[active='true']:hidden" icon={sunIcon} />
				</button>
				<button
					type="button"
					class="btn btn-lg btn-circle col-start-1 group data-[active='true']:btn-active"
					onClick={handleFanButtonClick}
					data-active={!!fanState?.fan}
				>
					<Icon
						class="group-data-[active='false']:hidden"
						icon={sunFilledIcon}
					/>
					<Icon class="group-data-[active='true']:hidden" icon={sunIcon} />
				</button>
			</div>
		</div>
	)
}
